#pragma warning(disable: 4308)

#include "Client.h"

#include "Application.h"
#include "MenuState.h"
#include "PlayState.h"

namespace LC
{

	Client Client::client_;

	Client::Client() : connected_(false) {}

	Client::~Client()
	{ }

	tcp::socket& Client::socket()
	{
		return *socket_;
	}

	void Client::write_chat_message(std::string text)
	{
		write(text, CHAT_MESSAGE);
	}

	void Client::write_name_message()
	{
		write(client_name_, SET_NAME_MESSAGE);
	}

	void Client::write_disconnect_message()
	{
		strand_->post(boost::bind(&Client::do_write_disconnect_message, this));

		//strand_->post(boost::bind(&GameState::SetShutDown, TheApplication.getGameState(), true));
		connected_ = false;
	}

	// WHY??
	void Client::do_write_disconnect_message()
	{
		int i = 0;
		write(i, CLIENT_DISCONNECT_MESSAGE);
	}

	void Client::write_button_pressed_message(std::string button)
	{
		write(button, CLIENT_BUTTON_PRESSED_MESSAGE);
	}

	void Client::write_button_released_message(std::string button)
	{
		write(button, CLIENT_BUTTON_RELEASED_MESSAGE);
	}

	void Client::write_move_direction_message(Ogre::Vector3 direction)
	{
		std::vector<float> dir;
		dir.push_back(direction.x);
		dir.push_back(direction.y);
		dir.push_back(direction.z);
		write(dir, CLIENT_MOVE_DIRECTION_MESSAGE);
	}

	void Client::write_keep_alive_message()
	{
		int placeholder = 0;
		write(placeholder, KEEP_ALIVE_MESSAGE);
	}

	template<typename T>
	void Client::write(T &t, MessageType type)
	{
		void (Client::*f)(T&, MessageType) = &Client::do_write<T>;
		io_service_->post(boost::bind(f, this, t, type));
	}

	template<typename T>
	void Client::do_write(T &t, MessageType type)
	{
		// Serialize the data first so we know how large it is.
		std::ostringstream archive_stream;
		boost::archive::text_oarchive archive(archive_stream);
		archive << t;
		outbound_data_queue_.push_back(archive_stream.str());

		// Format the header.
		std::ostringstream header_stream;
		header_stream << std::setw(header_length)
			<< std::hex << outbound_data_queue_.back().size();
		if (!header_stream || header_stream.str().size() != header_length)
		{
			// Something went wrong, inform the caller.
			boost::system::error_code error(boost::asio::error::invalid_argument);
			std::cout << error.message() << std::endl;
			return;
		}

		outbound_header_queue_.push_back(header_stream.str());

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

		outbound_message_type_queue_.push_back(message_type_stream.str());

		// Write the serialized data to the socket. We use "gather-write" to send
		// both the header and the data in a single write operation.
		std::vector<boost::asio::const_buffer> buffers;
		buffers.push_back(boost::asio::buffer(outbound_header_queue_.back()));
		buffers.push_back(boost::asio::buffer(outbound_message_type_queue_.back()));
		buffers.push_back(boost::asio::buffer(outbound_data_queue_.back()));
		
		bool write_in_progress = !write_queue_.empty();
		write_queue_.push_back(buffers);
		if (!write_in_progress)
		{
			boost::asio::async_write(*socket_, write_queue_.front(),
				boost::bind(&Client::handle_write, this,
					boost::asio::placeholders::error));
		}
	}

	void Client::handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			write_queue_.pop_front();
			outbound_header_queue_.pop_front();
			outbound_message_type_queue_.pop_front();
			outbound_data_queue_.pop_front();
			if (!write_queue_.empty())
			{
				boost::asio::async_write(*socket_, write_queue_.front(),
				boost::bind(&Client::handle_write, this,
					boost::asio::placeholders::error));
			}
		}
		else
		{
			std::cout << "ERROR" << std::endl;
		}
	}

	void Client::close()
	{
		io_service_->post(boost::bind(&Client::do_close, this));
	}

	std::string Client::get_client_name()
	{
		return client_name_;
	}
	void Client::reset(boost::shared_ptr<boost::asio::io_service> io_service,
		tcp::resolver::iterator endpoint_iterator, std::string name)
	{
		outbound_header_queue_.clear();
		outbound_message_type_queue_.clear();
		outbound_data_queue_.clear();

		write_queue_.clear();

		io_service_ = io_service;

		socket_.reset(new tcp::socket(*io_service_));

		strand_.reset(new boost::asio::io_service::strand(*io_service));

		client_name_ = name;

		boost::asio::async_connect(*socket_, endpoint_iterator,
			boost::bind(&Client::handle_connect, this, 
				boost::asio::placeholders::error));

		keep_alive_timer_ = 0.f;
	}

	bool Client::frameRenderingQueued(const Ogre::FrameEvent &evt)
	{
		if(connected_)
		{
			// Disconnect.
			if(keep_alive_timer_ > keep_alive_failure_interval)
			{
				write_disconnect_message();
				if(TheApplication.getGameState() == LC::MenuState::getSingletonPtr())
					CEGUI::WindowManager::getSingletonPtr()
						->getWindow("LIGHTCYCLEMENU/Lobby/ConnectedIP")->setText("DISCONNECTED");
			}

			// Send Keep Alive Message.
			if(keep_alive_timer_ > keep_alive_send_interval)
				write_keep_alive_message();

			keep_alive_timer_ += evt.timeSinceLastFrame;
		}

		return true;
	}

	void Client::handle_connect(const boost::system::error_code& error)
	{
		if (!error)
		{
			boost::asio::async_read(*socket_,
				boost::asio::buffer(inbound_header_), 
				boost::bind(&Client::handle_read_header, this,
					boost::asio::placeholders::error));

			write_name_message();
		}
		else
		{
			std::cerr << error.message() << std::endl;
		}
	}

	void Client::handle_read_header(const boost::system::error_code &e)
	{
		if(e)
		{
			boost::system::error_code error(boost::asio::error::invalid_argument);
			std::cout << "ERROR" << std::endl;
		}
		else
		{
			// Determine the length of the serialized data.
			std::istringstream is(std::string(inbound_header_, header_length));
			std::size_t inbound_data_size = 0;
			if (!(is >> std::hex >> inbound_data_size))
			{
				// Header doesn't seem to be valid. Inform the caller.
				boost::system::error_code error(boost::asio::error::invalid_argument);
				std::cout << "ERROR" << std::endl;
				return;
			}

			// Start an asynchronous call to receive the data.
			inbound_data_.resize(inbound_data_size);

			boost::asio::async_read(*socket_, boost::asio::buffer(inbound_message_type_),
					boost::bind(&Client::handle_read_message_type, this,
						boost::asio::placeholders::error));			
		}
	}

	void Client::handle_read_message_type(const boost::system::error_code &error)
	{
		if(error)
		{
			boost::system::error_code error(boost::asio::error::invalid_argument);
			std::cout << "ERROR" << std::endl;
		}
		else
		{
			// Determine the message type.
			std::istringstream is(std::string(inbound_message_type_, message_type_length));
			
			std::size_t message_type = 0;
			//if (!(is >> std::dec >> message_type))
			if (!(is >> std::hex >> message_type))
			{
				// Header doesn't seem to be valid. Inform the caller.
				boost::system::error_code error(boost::asio::error::invalid_argument);
				std::cout << "ERROR" << std::endl;
				return;
			}
			
			switch(message_type)
			{
			case CONNECTION_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_connection_message, this,
						boost::asio::placeholders::error));
				break;
			case CHAT_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_chat_message, this,
						boost::asio::placeholders::error));
				break;
			case SERVER_FULL_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_server_full_message, this,
						boost::asio::placeholders::error));
				break;
			case SERVER_CLOSE_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_server_close_message, this,
						boost::asio::placeholders::error));
				break;
			case SET_NAME_MESSAGE:
				// ON SERVER
				break;
			case GET_NAMES_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_get_names_message, this,
						boost::asio::placeholders::error));
				break;
			case CLIENT_DISCONNECT_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_get_names_message, this,
						boost::asio::placeholders::error));
				break;
			case GAME_START_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_game_start_message, this,
						boost::asio::placeholders::error));
				break;
			case CLIENT_BUTTON_PRESSED_MESSAGE:
				// ON SERVER
				break;
			case CLIENT_BUTTON_RELEASED_MESSAGE:
				// ON SERVER
				break;
			case SNAPSHOT_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_snapshot_message, this,
						boost::asio::placeholders::error));
				break;
			case CLIENT_CREATE_AVATAR_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_create_avatar_message, this,
						boost::asio::placeholders::error));
				break;
			case CLIENT_CREATE_WALL_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_create_wall_message, this,
						boost::asio::placeholders::error));
				break;
			case CLIENT_REMOVE_OBJECT_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_remove_client_object_message, this,
						boost::asio::placeholders::error));
				break;
			case CLIENT_A_PLAYER_IS_DEAD_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_a_player_is_dead_message, this,
						boost::asio::placeholders::error));
				break;
			case KEEP_ALIVE_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_keep_alive_message, this,
						boost::asio::placeholders::error));
				break;
			case RESET_GAME_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_reset_game_message, this,
						boost::asio::placeholders::error));
				break;
			case VICTORY_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_victory_message, this,
						boost::asio::placeholders::error));
				break;
			case SCORE_MESSAGE:
				boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Client::handle_score_message, this,
						boost::asio::placeholders::error));
				break;
			default:
				break;
			}
		}
	}

	void Client::handle_connection_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			int placeholder;
			handle_read_body(e, placeholder);
			connected_ = true;
			if(TheApplication.getGameState() == LC::MenuState::getSingletonPtr())
				LC::MenuState::getSingletonPtr()->ShowLobby();

		}
	}

	void Client::handle_chat_message(const boost::system::error_code& e)
	{
		if(!e)
		{
			std::string text;
			handle_read_body(e, text);
			TheApplication.getGameState()->HandleChatMessage(text);
		}
	}

	void Client::handle_get_names_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			std::vector<std::string> names;
			handle_read_body(e, names);
			TheApplication.UpdateParticipants(names);
		}
	}

	void Client::handle_server_full_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			int placeholder;
			handle_read_body(e, placeholder);
		}

		socket_->shutdown(boost::asio::socket_base::shutdown_both);
		if(TheApplication.getGameState() == LC::MenuState::getSingletonPtr())
		{
			LC::MenuState::getSingletonPtr()->ShowNAMEIPMenu();
			CEGUI::WindowManager::getSingletonPtr()->getWindow("LIGHTCYCLEMENU/IP/EditBox")->setText("Server Full");
			CEGUI::WindowManager::getSingletonPtr()->getWindow("LIGHTCYCLEMENU/Name/EditBox")->setText(client_name_);
		}
	}

	void Client::handle_server_close_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			int placeholder;
			handle_read_body(e, placeholder);
			
			if(socket_)
			{
				socket_->shutdown(boost::asio::socket_base::shutdown_send);
				CEGUI::WindowManager::getSingletonPtr()
					->getWindow("LIGHTCYCLEMENU/Lobby/ConnectedIP")->setText("DISCONNECTED");
			}
		}
	}

	// Initial state of map, players, etc. Needs to be sent.
	void Client::handle_game_start_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			int placeholder = 0;
			handle_read_body(e, placeholder);
			if(TheApplication.getGameState() == LC::MenuState::getSingletonPtr())
				TheApplication.ChangeState(LC::PlayState::getSingletonPtr());
		}
	}

	void Client::handle_snapshot_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			std::vector<SnapShotObject> snapshot_instances;
			handle_read_body(e, snapshot_instances);
			if(TheApplication.getGameState() == LC::PlayState::getSingletonPtr())
				LC::PlayState::getSingletonPtr()->HandleSnapShot(snapshot_instances);
		}
	}

	void Client::handle_create_avatar_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			SnapShotObject avatar;
			handle_read_body(e, avatar);
			if(TheApplication.getGameState() == LC::PlayState::getSingletonPtr())
				LC::PlayState::getSingletonPtr()->HandleCreateAvatar(avatar);
		}
	}

	void Client::handle_create_wall_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			SnapShotObject wall;
			handle_read_body(e, wall);
			if(TheApplication.getGameState() == LC::PlayState::getSingletonPtr())
				LC::PlayState::getSingletonPtr()->ClientCreateWall(wall);
		}
	}

	void Client::handle_remove_client_object_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			std::string name;
			handle_read_body(e, name);
			if(TheApplication.getGameState() == LC::PlayState::getSingletonPtr())
				LC::PlayState::getSingletonPtr()->RemoveClientObject(name);
		}
	}

	void Client::handle_a_player_is_dead_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			std::string player_name;
			handle_read_body(e, player_name);
			if(TheApplication.getGameState() == LC::PlayState::getSingletonPtr())
				LC::PlayState::getSingletonPtr()->HandlePlayerIsDead(player_name);
		}
	}

	void Client::handle_keep_alive_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			int placeholder;
			handle_read_body(e, placeholder);
			keep_alive_timer_ = 0.f;
		}
	}

	void Client::handle_reset_game_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			int placeholder;
			handle_read_body(e, placeholder);
			if(TheApplication.getGameState() == LC::PlayState::getSingletonPtr())
				LC::PlayState::getSingletonPtr()->ResetGame();
		}
	}

	void Client::handle_victory_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			std::string name;
			handle_read_body(e, name);
			if(TheApplication.getGameState() == LC::PlayState::getSingletonPtr())
				LC::PlayState::getSingletonPtr()->HandleVictory(name);
		}
	}

	void Client::handle_score_message(const boost::system::error_code &e)
	{
		if(!e)
		{
			std::vector<std::string> player_scores;
			handle_read_body(e, player_scores);
			if(TheApplication.getGameState() == LC::PlayState::getSingletonPtr())
				LC::PlayState::getSingletonPtr()->SetScore(player_scores);
		}
	}

	template <typename T>
	void Client::handle_read_body(const boost::system::error_code& e, T& t)
	{
		if(e)
		{
			boost::system::error_code error(boost::asio::error::invalid_argument);
			std::cout << e.message() << std::endl;
		}
		else
		{
			// Extract the data structure from the data just received.
			try
			{
				std::string archive_data(&inbound_data_[0], inbound_data_.size());
				std::istringstream archive_stream(archive_data);
				boost::archive::text_iarchive archive(archive_stream);

				archive >> t;
			}
			catch (std::exception& e)
			{
				// Unable to decode data.
				boost::system::error_code error(boost::asio::error::invalid_argument);
				std::cout << e.what() << std::endl;
				return;
			}

			boost::asio::async_read(*socket_,
				boost::asio::buffer(inbound_header_), 
				boost::bind(&Client::handle_read_header, this,
					boost::asio::placeholders::error));
		}
	}

	void Client::do_close()
	{
		//write_disconnect_message();
	}

	bool Client::is_connected()
	{
		return connected_;
	}
}