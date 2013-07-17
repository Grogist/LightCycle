#ifndef _SERVER_H_
#define _SERVER_H_

#include <OgreTimer.h>
#include <OgreFrameListener.h>

#include "Participant.h"

using boost::asio::ip::tcp;

namespace LC
{
	class Server : public Ogre::FrameListener
	{
	public:
		~Server();
		void start_accept();
		void handle_accept(participant_ptr participant, const boost::system::error_code& error);
		// This function should not exist!
		void write_all_chat_messages_to_all();
		// Used when Server wants to send messages.
		void write_chat_message(std::string text);
		void write_get_names_message();
		void write_server_close_message();
		void do_server_close_message();
		// Initial state of map, players, etc. Needs to be sent.
		void write_game_start_message();
		//void write_game_start_message(std::vector<something> Parameters);
		void write_snapshot_message(std::vector<SnapShotObject> snapshot_instances);
		void write_create_avatar_message(std::vector<SnapShotObject> player);
		void write_create_wall_message(SnapShotObject wall);
		void write_remove_client_object_message(std::string temp_wall_name);
		void write_a_player_is_dead_message(std::string player_name);
		void write_reset_game_message();
		void write_victory_message(std::string name);
		void write_score_message(std::vector<std::string> player_scores);
		
		void leave(participant_ptr participant);
		void close();

		// Not Implemented
		std::string get_server_name();
		std::set<participant_ptr> *get_all_participants();
		std::vector<std::string> get_all_participant_names();

		// Assumption that Address remains constant through the duration of running
		//  the program.
		void reset(boost::shared_ptr<boost::asio::io_service> io_service, std::string name);

		static Server* getSingletonPtr() { return &server_; }

	protected:
		virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt);

	private:
		Server();

		template<typename T>
		void write(T& t, MessageType type);
		template<typename T>
		void do_write(T &t, MessageType type);
		void do_close();
		
		void leaveAll();

		boost::shared_ptr<boost::asio::io_service> io_service_;
		boost::shared_ptr<tcp::acceptor> acceptor_;
		boost::shared_ptr<boost::asio::io_service::strand> strand_;

		std::set<participant_ptr> participants_;
		enum { max_recent_chat_msgs = 100 };
		std::deque<std::string> recent_chat_msgs_;

		enum { max_participants = 4 };

		std::string server_name_;

		enum { snapshot_interval = 75 }; //Milliseconds
		
		Ogre::Timer snapshot_timer_;

		static Server server_;
	};

}

#endif