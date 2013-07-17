#ifndef _PARTICIPANT_H_
#define _PARTICIPANT_H_

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <OGRE/OgreTimer.h>

#include "MessageTypes.h"
#include "SnapShotObject.h"
#include "GameObject.h"

using boost::asio::ip::tcp;

namespace LC
{
	typedef std::deque<std::vector<boost::asio::const_buffer>> write_buffer_queue;

	class Participant : public boost::enable_shared_from_this<Participant>
	{
	public:
		Participant(boost::asio::io_service& io_service);
		~Participant();
		tcp::socket& socket();
		void start();
		void write_connection_message();
		void write_chat_messages(std::string text, MessageType type);
		void write_create_avatar_message(LC::SnapShotObject avatar, MessageType type);
		void write_keep_alive_message();
		void write_server_full_message();
		// Used for sending message to specific participants.
		template<typename T>
		void write(T& t, MessageType type);
		// Used for broadcast messages. Server encodes the message to save computation.
		void write(std::string header, std::string message_type, std::string body);

		void handle_read_header(const boost::system::error_code &e);
		void handle_read_message_type(const boost::system::error_code &e);

		void handle_chat_message(const boost::system::error_code &e);
		void handle_client_disconnect_message(const boost::system::error_code& e);
		void handle_set_name_message(const boost::system::error_code &e);
		void handle_client_button_pressed_message(const boost::system::error_code& e);
		void handle_client_button_released_message(const boost::system::error_code& e);
		void handle_client_move_direction_message(const boost::system::error_code& e);
		void handle_keep_alive_message(const boost::system::error_code& e);

		template <typename T>
		void handle_read_body(const boost::system::error_code& e, T& t);
		void handle_write(const boost::system::error_code& error);

		std::string get_participant_name();
		void set_participant_name(std::string name);

		float keep_alive_timer_;

		LC::ServerPlayer *player_ptr_;

		int score_;

	private:
		tcp::socket socket_;

		/// Holds an inbound header.
		char inbound_header_[header_length];
		
		char inbound_message_type_[message_type_length];
		std::vector<char> inbound_data_;

		std::deque<std::string> outbound_header_queue_;
		std::deque<std::string> outbound_message_type_queue_;
		std::deque<std::string> outbound_data_queue_;

		write_buffer_queue write_queue_;

		std::string Participant_name_;
	};

	typedef boost::shared_ptr<Participant> participant_ptr;
}

#endif