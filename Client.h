#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <OgreTimer.h>
#include <OgreFrameListener.h>

#include "MessageTypes.h"

using boost::asio::ip::tcp;

namespace LC
{	
	typedef std::deque<std::vector<boost::asio::const_buffer>> write_buffer_queue;

	class Client : public Ogre::FrameListener
	{
	public:
		~Client();
		tcp::socket& socket();

		void write_chat_message(std::string text);
		void write_name_message();
		void write_disconnect_message();
		void do_write_disconnect_message();
		void write_button_pressed_message(std::string button);
		void write_button_released_message(std::string button);
		void write_move_direction_message(Ogre::Vector3 direction);
		void write_keep_alive_message();

		void close();

		bool is_connected();

		std::string get_client_name();

		void reset(boost::shared_ptr<boost::asio::io_service> io_service,
			tcp::resolver::iterator endpoint_iterator, std::string name);

		static Client* getSingletonPtr() { return &client_; }

	protected:
		virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt);

	private:
		// A call to reset is needed to start async connect.
		Client();

		template<typename T>
		void write(T &t, MessageType type);
		template<typename T>
		void do_write(T &t, MessageType type);
		void handle_connect(const boost::system::error_code &e);
		void handle_read_header(const boost::system::error_code &e);
		void handle_read_message_type(const boost::system::error_code &e);
		void handle_connection_message(const boost::system::error_code &e);
		void handle_chat_message(const boost::system::error_code &e);
		void handle_get_names_message(const boost::system::error_code &e);
		void handle_server_full_message(const boost::system::error_code &e);
		void handle_server_close_message(const boost::system::error_code &e);
		// Initial state of map, players, etc. Needs to be sent.
		void handle_game_start_message(const boost::system::error_code &e);
		void handle_snapshot_message(const boost::system::error_code &e);
		void handle_create_avatar_message(const boost::system::error_code &e);
		void handle_create_wall_message(const boost::system::error_code &e);
		void handle_remove_client_object_message(const boost::system::error_code &e);
		void handle_a_player_is_dead_message(const boost::system::error_code &e);
		void handle_keep_alive_message(const boost::system::error_code &e);
		void handle_reset_game_message(const boost::system::error_code &e);
		void handle_victory_message(const boost::system::error_code &e);
		void handle_score_message(const boost::system::error_code &e);
		template <typename T>
		void handle_read_body(const boost::system::error_code& e, T& t);
		void handle_write(const boost::system::error_code& e);
		void do_close();

		float keep_alive_timer_;

		boost::shared_ptr<boost::asio::io_service> io_service_;
	public:
		boost::shared_ptr<tcp::socket> socket_;
	private:
		boost::shared_ptr<boost::asio::io_service::strand> strand_;
		
		/// Holds an inbound header.
		char inbound_header_[header_length];
		char inbound_message_type_[message_type_length];
		std::vector<char> inbound_data_;

		std::deque<std::string> outbound_header_queue_;
		std::deque<std::string> outbound_message_type_queue_;
		std::deque<std::string> outbound_data_queue_;

		write_buffer_queue write_queue_;

		enum { max_recent_chat_msgs = 100 };
		std::deque<std::string> recent_chat_msgs_;

		bool connected_;

		std::string client_name_;

		static Client client_;
	};

}

#endif