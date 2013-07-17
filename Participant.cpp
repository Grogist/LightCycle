#pragma warning(disable: 4308)

#include "Participant.h"

#include "Application.h"
#include "Server.h"
#include "Client.h"

namespace LC
{
	Participant::Participant(boost::asio::io_service& io_service)
		: socket_(io_service), player_ptr_(nullptr), score_(0)
	{ }

	Participant::~Participant()
	{ }

	tcp::socket& Participant::socket()
	{
		return socket_;
	}

	void Participant::start()
	{
		boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
			boost::bind(&Participant::handle_read_header, shared_from_this(),
				boost::asio::placeholders::error));

		keep_alive_timer_ = 0.f;
	}

	void Participant::write_connection_message()
	{
		int placeholder = 0;
		write(placeholder, CONNECTION_MESSAGE);
	}

	void Participant::write_chat_messages(std::string text, MessageType type)
	{
		write(text, type);
	}

	void Participant::write_create_avatar_message(LC::SnapShotObject avatar, MessageType type)
	{
		write(avatar, type);
	}

	void Participant::write_keep_alive_message()
	{
		int placeholder = 0;
		write(placeholder, KEEP_ALIVE_MESSAGE);
	}

	void Participant::write_server_full_message()
	{
		int placeholder = 0;
		write(placeholder, SERVER_FULL_MESSAGE);
	}

	// Used for sending message to specific participants.
	template<typename T>
	void Participant::write(T& t, MessageType type)
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
			boost::asio::async_write(socket_, write_queue_.front(),
				boost::bind(&Participant::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
		}
	}

	// Used for broadcast messages. Server encodes the message to save computation.
	void Participant::write(std::string header, std::string message_type, std::string body)
	{
		outbound_header_queue_.push_back(header);
		outbound_message_type_queue_.push_back(message_type);
		outbound_data_queue_.push_back(body);

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
			boost::asio::async_write(socket_, write_queue_.front(),
				boost::bind(&Participant::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
		}
	}

	void Participant::handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			write_queue_.pop_front();
			outbound_header_queue_.pop_front();
			outbound_message_type_queue_.pop_front();
			outbound_data_queue_.pop_front();
			if (!write_queue_.empty())
			{
				boost::asio::async_write(socket_, write_queue_.front(),
				boost::bind(&Participant::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
			}
		}
		else
		{
			std::cout << "ERROR" << std::endl;
		}
	}

	void Participant::handle_read_header(const boost::system::error_code& error)
	{
		if(error)
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
			
			boost::asio::async_read(socket_, boost::asio::buffer(inbound_message_type_),
					boost::bind(&Participant::handle_read_message_type, this,
						boost::asio::placeholders::error));
		}
	}

	void Participant::handle_read_message_type(const boost::system::error_code &error)
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
				break;
			case CHAT_MESSAGE:
				boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Participant::handle_chat_message, this,
						boost::asio::placeholders::error));
				break;
			case SERVER_FULL_MESSAGE:
				// ON CLIENT
				break;
			case SERVER_CLOSE_MESSAGE:
				break;
			case SET_NAME_MESSAGE:
				boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Participant::handle_set_name_message, this,
						boost::asio::placeholders::error));
				break;
			case GET_NAMES_MESSAGE:
				// ON CLIENT
				break;
			case CLIENT_DISCONNECT_MESSAGE:
				boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Participant::handle_client_disconnect_message, this,
						boost::asio::placeholders::error));
				break;
			case CLIENT_BUTTON_PRESSED_MESSAGE:
				boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Participant::handle_client_button_pressed_message, this,
						boost::asio::placeholders::error));
				break;
			case CLIENT_BUTTON_RELEASED_MESSAGE:
				boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Participant::handle_client_button_released_message, this,
						boost::asio::placeholders::error));
				break;
			case CLIENT_MOVE_DIRECTION_MESSAGE:
				boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Participant::handle_client_move_direction_message, this,
						boost::asio::placeholders::error));
				break;
			case SNAPSHOT_MESSAGE:
				break;
			case KEEP_ALIVE_MESSAGE:
				boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
					boost::bind(&Participant::handle_keep_alive_message, this,
						boost::asio::placeholders::error));
				break;
			default:
				break;
			}
		}
	}

	void Participant::handle_chat_message(const boost::system::error_code& e)
	{
		if(!e)
		{
			std::string text;
			handle_read_body(e, text);
			Server::getSingletonPtr()->write_chat_message(text);
		}	
	}

	void Participant::handle_set_name_message(const boost::system::error_code& e)
	{
		if(!e)
		{
			std::string text;
			handle_read_body(e, text);
			set_participant_name(text);
			Server::getSingletonPtr()->write_get_names_message();
		}
		else
		{
			int i =0;
		}
		
	}

	void Participant::handle_client_disconnect_message(const boost::system::error_code& e)
	{
		if(!e)
		{
			int i;
			handle_read_body(e, i);
			Server::getSingletonPtr()->leave(shared_from_this());
		}
		else
		{
			int i = 0;
		}

	}

	// ONLY OCCURS IN PLAYSTATE.
	void Participant::handle_client_button_pressed_message(const boost::system::error_code& e)
	{
		if(!e)
		{
			std::string button;
			handle_read_body(e, button);
			if(player_ptr_)
				player_ptr_->KeyPressed(button);
		}
	}

	// ONLY OCCURS IN PLAYSTATE.
	void Participant::handle_client_button_released_message(const boost::system::error_code& e)
	{
		if(!e)
		{
			std::string button;
			handle_read_body(e, button);
			if(player_ptr_)
				player_ptr_->KeyReleased(button);
		}
	}

	void Participant::handle_client_move_direction_message(const boost::system::error_code& e)
	{
		if(!e)
		{
			std::vector<float> direction;
			handle_read_body(e, direction);
			if(player_ptr_)
				player_ptr_->SetMoveDirection(direction);
		}
	}

	void Participant::handle_keep_alive_message(const boost::system::error_code& e)
	{
		if(!e)
		{
			int placeholder;
			handle_read_body(e, placeholder);
			keep_alive_timer_ = 0.f;
			write_keep_alive_message();
		}
	}

	template <typename T>
	void Participant::handle_read_body(const boost::system::error_code& e, T& t)
	{
		if(e)
		{
			boost::system::error_code error(boost::asio::error::invalid_argument);
			std::cout << "ERROR" << std::endl;
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
			catch (std::exception& error)
			{
				// Unable to decode data.
				std::cout << error.what() << std::endl;
				return;
			}

			boost::asio::async_read(socket_,
				boost::asio::buffer(inbound_header_), 
				boost::bind(&Participant::handle_read_header, shared_from_this(),
					boost::asio::placeholders::error));
			
		}
	}

	std::string Participant::get_participant_name()
	{
		return Participant_name_;
	}

	void Participant::set_participant_name(std::string name)
	{
		Participant_name_ = name;
	}
}
