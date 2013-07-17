#pragma warning(disable: 4308)

#include "Server.h"

#include "Application.h"
#include "PlayState.h"

namespace LC
{
	Server Server::server_;

	Server::Server()
	{
		snapshot_timer_.reset();
	}

	Server::~Server()
	{ }

	void Server::start_accept()
	{
		participant_ptr new_session(new Participant(*io_service_));
		acceptor_->async_accept(new_session->socket(), boost::bind(&Server::handle_accept,
			this, new_session, boost::asio::placeholders::error));
	}

	void Server::handle_accept(participant_ptr participant, const boost::system::error_code& error)
	{
		if (!error)
		{
			participants_.insert(participant);

			if(participants_.size() > max_participants)
			{
				participant->write_server_full_message();

				// participant is automatically be removed from participants_ when 
				//  it's keep alive timer expires.
				
				return;
			}

			participant->write_connection_message();

			std::deque<std::string>::iterator itr = recent_chat_msgs_.begin();
			while( itr != recent_chat_msgs_.end())
			{
				participant->write_chat_messages((*itr), CHAT_MESSAGE);
				++itr;
			}

			participant->start();
		}

		start_accept();

		if(error)
		{
			std::cout << error.message();
		}		
	}

	void Server::write_all_chat_messages_to_all()
	{
		for(unsigned int i = 0; i < recent_chat_msgs_.size(); i++)
		{
			write(recent_chat_msgs_.at(i), CHAT_MESSAGE);
		}
	}

	void Server::write_chat_message(std::string text)
	{
		recent_chat_msgs_.push_back(text);
		while (recent_chat_msgs_.size() > max_recent_chat_msgs && !recent_chat_msgs_.empty())
			recent_chat_msgs_.pop_front();

		write(text, CHAT_MESSAGE);
	}

	void Server::write_get_names_message()
	{
		std::vector<std::string> names = get_all_participant_names();

		write(names, GET_NAMES_MESSAGE);
	}

	void Server::write_server_close_message()
	{
		// NOT NEEDED!!!! WHY NOT NEEDED?
		strand_->post(boost::bind(&Server::do_server_close_message, this));
	}

	void Server::do_server_close_message()
	{
		int placeholder = 0;
		write(placeholder, SERVER_CLOSE_MESSAGE);
	}

	// Initial state of map, players, etc. Needs to be sent.
	void Server::write_game_start_message()
	{
		int placeholder = 0;
		write(placeholder, GAME_START_MESSAGE);
	}

	void Server::write_snapshot_message(std::vector<SnapShotObject> snapshot_instances)
	{
		std::vector<SnapShotObject> snapshot_instances_copy = snapshot_instances; // WHY??
		write(snapshot_instances_copy, SNAPSHOT_MESSAGE);
	}

	void Server::write_create_avatar_message(std::vector<SnapShotObject> player)
	{
		std::set<participant_ptr>::iterator participants_itr = participants_.begin();
		std::vector<SnapShotObject>::iterator player_itr = player.begin();
		for(participants_itr, player_itr; participants_itr != participants_.end()
			&& player_itr != player.end(); ++participants_itr, ++player_itr)
		{
			participants_itr->get()->write_create_avatar_message(*player_itr,
				CLIENT_CREATE_AVATAR_MESSAGE);
		}
	}

	void Server::write_create_wall_message(SnapShotObject wall)
	{
		write(wall, CLIENT_CREATE_WALL_MESSAGE);
	}

	void Server::write_remove_client_object_message(std::string name)
	{
		write(name, CLIENT_REMOVE_OBJECT_MESSAGE);
	}

	void Server::write_a_player_is_dead_message(std::string player_name)
	{
		write(player_name, CLIENT_A_PLAYER_IS_DEAD_MESSAGE);
	}

	void Server::write_reset_game_message()
	{
		int placeholder = 0;
		write(placeholder, RESET_GAME_MESSAGE);
	}

	void Server::write_victory_message(std::string name)
	{
		write(name, VICTORY_MESSAGE);
	}

	void Server::write_score_message(std::vector<std::string> player_scores)
	{
		write(player_scores, SCORE_MESSAGE);
	}

	template<typename T>
	void Server::write(T& t, MessageType type)
	{
		void (Server::*f)(T&, MessageType) = &Server::do_write<T>;
		io_service_->post(boost::bind(f, this, t, type));
	}
	
	template<typename T>
	void Server::do_write(T& t, MessageType type)
	{
		// Serialize the data first so we know how large it is.
		std::ostringstream archive_stream;
		boost::archive::text_oarchive archive(archive_stream);
		archive << t;

		std::string body = archive_stream.str();

		// Format the header.
		std::ostringstream header_stream;
		header_stream << std::setw(header_length)
			<< std::hex << body.size();
		if (!header_stream || header_stream.str().size() != header_length)
		{
			// Something went wrong, inform the caller.
			boost::system::error_code error(boost::asio::error::invalid_argument);
			std::cout << error.message() << std::endl;
			return;
		}

		std::string header = header_stream.str();

		// Format the message type.
		std::ostringstream message_type_stream;
		message_type_stream << std::setw(message_type_length)
			<< std::hex << type;
		if (!message_type_stream || message_type_stream.str().size() != message_type_length)
		{
			// Something went wrong, inform the caller.
			boost::system::error_code error(boost::asio::error::invalid_argument);
			std::cout << error.message() << std::endl;
			return;
		}

		std::string message_type = message_type_stream.str();

		std::set<participant_ptr>::iterator itr = participants_.begin();
		for(itr; itr != participants_.end(); itr++)
		{
			(*itr)->write(header, message_type, body);
		}
	}

	void Server::close()
	{ 
		io_service_->post(boost::bind(&Server::do_close, this));
	}

	static bool occurred = false;

	void Server::reset(boost::shared_ptr<boost::asio::io_service> io_service, std::string name)
	{
		io_service_ = io_service;

		tcp::endpoint endpoint(tcp::v4(), TheApplication.getPortNumber());

		acceptor_.reset(new tcp::acceptor(*io_service, endpoint));
		strand_.reset(new boost::asio::io_service::strand(*io_service));

		leaveAll();

		server_name_ = name;

		recent_chat_msgs_.clear();

		start_accept();
	}

	bool Server::frameRenderingQueued(const Ogre::FrameEvent &evt)
	{
		if(TheApplication.IsHost())
		{
			if(TheApplication.getGameState() == LC::PlayState::getSingletonPtr())
			{
				if(snapshot_timer_.getMilliseconds() > snapshot_interval)
				{
					std::vector<SnapShotObject> snapshot_instances;
				
					snapshot_instances = LC::PlayState::getSingletonPtr()->GetSnapShot();

					write_snapshot_message(snapshot_instances);
				}

			}

			std::set<participant_ptr>::iterator itr = participants_.begin();
			for(itr; itr != participants_.end(); ++itr)
			{
				itr->get()->keep_alive_timer_ += evt.timeSinceLastFrame;
				float time = itr->get()->keep_alive_timer_;
				if(time > keep_alive_failure_interval)
				{
					leave(itr->get()->shared_from_this());
					itr = participants_.begin();
				}
			}
		}

		return true;
	}

	void Server::leave(participant_ptr participant)
	{
		if(TheApplication.getGameState() == LC::PlayState::getSingletonPtr() &&
			participant->player_ptr_)
		{
			std::string name = participant->player_ptr_->m_Name;
			LC::PlayState::getSingletonPtr()->RemoveServerObject(name);
			write_remove_client_object_message(name);
		}

		participants_.erase(participant);

		write_get_names_message();
	}

	void Server::leaveAll()
	{
		participants_.clear();
	}

	std::string Server::get_server_name()
	{
		return server_name_;
	}

	// Not Implemented
	std::set<participant_ptr> *Server::get_all_participants()
	{
		return &participants_;
	}

	std::vector<std::string> Server::get_all_participant_names()
	{
		std::vector<std::string> participant_names;
		
		std::set<participant_ptr>::iterator itr;
		for(itr = participants_.begin(); itr != participants_.end(); itr++)
		{
			participant_names.push_back((*itr)->get_participant_name());
		}
		return participant_names;
	}

	void Server::do_close()
	{
		acceptor_->close();
		participants_.clear();
	}
}
