#include <iostream>

#include <restinio/all.hpp>

#include <json_dto/pub.hpp>

struct weather_t
{
	weather_t() = default;

	weather_t(
		std::string ID,
		std::string Date,
		std::string Time,
		std::string Name,
		std::string Lat,
		std::string Lon,
		std::string Temperature,
		std::string Humidity )
		:	m_ID{ std::move( ID ) }
		,	m_Date{ std::move( Date ) }
		,	m_Time{ std::move( Time ) }
		,	m_Name{ std::move ( Name ) }
		,	m_Lat{ std::move ( Lat ) }
		,	m_Lon{ std::move ( Lon ) }
		,	m_Temperature{ std::move( Temperature ) }
		,	m_Humidity { std::move( Humidity ) }
	{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "ID", m_ID )
			& json_dto::mandatory( "Date", m_Date )
			& json_dto::mandatory( "Time", m_Time )
			& json_dto::mandatory( "Name", m_Name )
			& json_dto::mandatory( "Lat", m_Lat )
			& json_dto::mandatory( "Lon", m_Lon )
			& json_dto::mandatory( "Temperature", m_Temperature )
			& json_dto::mandatory( "Humidity", m_Humidity );
	}

	std::string m_ID;
	std::string m_Date;
	std::string m_Time;
	std::string m_Name;
	std::string m_Lat;
	std::string m_Lon;
	std::string m_Temperature;
	std::string m_Humidity;
};

using weather_collection_t = std::vector< weather_t >;

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

class weathers_handler_t
{
public :
	explicit weathers_handler_t( weather_collection_t & weathers )
		:	m_weathers( weathers )
	{}

	weathers_handler_t( const weathers_handler_t & ) = delete;
	weathers_handler_t( weathers_handler_t && ) = delete;

	auto on_weathers_list(
		const restinio::request_handle_t& req, rr::route_params_t ) const
	{
		auto resp = init_resp( req->create_response() );

		resp.set_body(
			"Weather collection (Weather count: " +
				std::to_string( m_weathers.size() ) + ")\n" );

		for( std::size_t i = 0; i < m_weathers.size(); ++i )
		{
			const auto & b = m_weathers[ i ];
			resp.append_body( "Weather data " + std::to_string( i + 1 ) + ". \n" );
			resp.append_body( "ID: " + b.m_ID + "\n" );
			resp.append_body( "Tidspunkt (dato og klokkeslet):\n");
			resp.append_body( "		Dato:" + b.m_Date + "\n");
			resp.append_body( "		Klokkeslet:" + b.m_Time + "\n");
			resp.append_body( "Sted:\n");
			resp.append_body( "		Navn: " + b.m_Name + "\n");
			resp.append_body( "		Lat: " + b.m_Lat + "\n");
			resp.append_body( "		Lon: " + b.m_Lon + "\n");
			resp.append_body( "Temperatur: " + b.m_Temperature + "\n" );
			resp.append_body( "Luftfugtighed: " + b.m_Humidity + "\n" );
		}

		return resp.done();
	}

	auto on_weather_get(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto weathernum = restinio::cast_to< std::uint32_t >( params[ "weathernum" ] );

		auto resp = init_resp( req->create_response() );

		if( 0 != weathernum && weathernum <= m_weathers.size() )
		{
			const auto & b = m_weathers[ weathernum - 1 ];
			resp.set_body(
				"Weather #" + std::to_string( weathernum ) + " is: " +
					b.m_Date + " [" + b.m_ID + "]\n" );
		}
		else
		{
			resp.set_body(
				"No weather with #" + std::to_string( weathernum ) + "\n" );
		}

		return resp.done();
	}

	auto on_ID_get(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		auto resp = init_resp( req->create_response() );
		try
		{
			auto ID = restinio::utils::unescape_percent_encoding( params[ "ID" ] );

			resp.set_body( "Weathers of " + ID + ":\n" );

			for( std::size_t i = 0; i < m_weathers.size(); ++i )
			{
				const auto & b = m_weathers[ i ];
				if( ID == b.m_ID )
				{
					resp.append_body( "Weather data " + std::to_string( i + 1 ) + ". \n" );
					resp.append_body( "ID: " + b.m_ID + "\n" );
					resp.append_body( "Tidspunkt (dato og klokkeslet):\n");
					resp.append_body( "		Dato:" + b.m_Date + "\n");
					resp.append_body( "		Klokkeslet:" + b.m_Time + "\n");
					resp.append_body( "Sted:\n");
					resp.append_body( "		Navn: " + b.m_Name + "\n");
					resp.append_body( "		Lat: " + b.m_Lat + "\n");
					resp.append_body( "		Lon: " + b.m_Lon + "\n");
					resp.append_body( "Temperatur: " + b.m_Temperature + "\n" );
					resp.append_body( "Luftfugtighed: " + b.m_Humidity + "\n" );
				}
			}
		}
		catch( const std::exception & )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_new_weather(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			m_weathers.emplace_back(
				json_dto::from_json< weather_t >( req->body() ) );
		}
		catch( const std::exception & /*ex*/ )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_weather_update(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto weathernum = restinio::cast_to< std::uint32_t >( params[ "weathernum" ] );

		auto resp = init_resp( req->create_response() );
		try
		{
			auto b = json_dto::from_json< weather_t >( req->body() );

			if( 0 != weathernum && weathernum <= m_weathers.size() )
			{
				m_weathers[ weathernum - 1 ] = b;
			}
			else
			{
				mark_as_bad_request( resp );
				resp.set_body( "No weather with #" + std::to_string( weathernum ) + "\n" );
			}
		}
		catch( const std::exception & /*ex*/ )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_weather_delete(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto weathernum = restinio::cast_to< std::uint32_t >( params[ "weathernum" ] );

		auto resp = init_resp( req->create_response() );

		if( 0 != weathernum && weathernum <= m_weathers.size() )
		{
			const auto & b = m_weathers[ weathernum - 1 ];
			resp.set_body(
				"Delete weather #" + std::to_string( weathernum ) + ": " +
					b.m_Date + "[" + b.m_ID + "]\n" );

			m_weathers.erase( m_weathers.begin() + ( weathernum - 1 ) );
		}
		else
		{
			resp.set_body(
				"No weather with #" + std::to_string( weathernum ) + "\n" );
		}

		return resp.done();
	}

private :
	weather_collection_t & m_weathers;

	template < typename RESP >
	static RESP
	init_resp( RESP resp )
	{
		resp
			.append_header( "Server", "RESTinio sample server /v.0.6" )
			.append_header_date_field()
			.append_header( "Content-Type", "text/plain; charset=utf-8" );

		return resp;
	}

	template < typename RESP >
	static void
	mark_as_bad_request( RESP & resp )
	{
		resp.header().status_line( restinio::status_bad_request() );
	}
};

auto server_handler( weather_collection_t & weather_collection )
{
	auto router = std::make_unique< router_t >();
	auto handler = std::make_shared< weathers_handler_t >( std::ref(weather_collection) );

	auto by = [&]( auto method ) {
		using namespace std::placeholders;
		return std::bind( method, handler, _1, _2 );
	};

	auto method_not_allowed = []( const auto & req, auto ) {
			return req->create_response( restinio::status_method_not_allowed() )
					.connection_close()
					.done();
		};

	// Handlers for '/' path.
	router->http_get( "/", by( &weathers_handler_t::on_weathers_list ) );
	router->http_post( "/", by( &weathers_handler_t::on_new_weather ) );

	// Disable all other methods for '/'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(), restinio::http_method_post() ),
			"/", method_not_allowed );

	// Handler for '/ID/:ID' path.
	router->http_get( "/ID/:ID", by( &weathers_handler_t::on_ID_get ) );

	// Disable all other methods for '/ID/:ID'.
	router->add_handler(
			restinio::router::none_of_methods( restinio::http_method_get() ),
			"/ID/:ID", method_not_allowed );

	// Handlers for '/:weathernum' path.
	router->http_get(
			R"(/:weathernum(\d+))",
			by( &weathers_handler_t::on_weather_get ) );
	router->http_put(
			R"(/:weathernum(\d+))",
			by( &weathers_handler_t::on_weather_update ) );
	router->http_delete(
			R"(/:weathernum(\d+))",
			by( &weathers_handler_t::on_weather_delete ) );

	// Disable all other methods for '/:weathernum'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(),
					restinio::http_method_post(),
					restinio::http_method_delete() ),
			R"(/:weathernum(\d+))", method_not_allowed );

	return router;
}

int main()
{
	using namespace std::chrono;

	try
	{
		using traits_t =
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				restinio::single_threaded_ostream_logger_t,
				router_t >;

		weather_collection_t weather_collection{
			{"1", "20211105", "12:15", "Aarhus N", "13.692", "19.438", "13.1", "70%"}
			/*{ "202001", "01 03 2022", "17 00", "Brabrand", "8 deg. C.", "40%" },
			{ "202002", "01 04 2022", "17 00", "Brabrand", "9 deg. C.", "50%" },
			{ "202003", "01 05 2022", "17 00", "Brabrand", "10 deg. C.", "60%" }*/
		};

		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "localhost" )
				.request_handler( server_handler( weather_collection ) )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
