#include <fmt/format.h>
#include <iostream>
#include <json_dto/pub.hpp>
#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

namespace rr   = restinio::router;
using router_t = rr::express_router_t<>;

namespace rws = restinio::websocket::basic;

using ws_registry_t = std::map<std::uint64_t, rws::ws_handle_t>;

using traits_t =
    restinio::traits_t<restinio::asio_timer_manager_t,
                       restinio::single_threaded_ostream_logger_t, router_t>;

// LAB 1
// struct for place
struct place_t
{
  place_t() = default;
  place_t(std::string placename, double lat, double lon)
      : m_placename{std::move(placename)},
        m_lat{std::move(lat)},
        m_lon{std::move(lon)}
  {
  }

  template <typename Json_Io> void json_io(Json_Io &io)
  {
    io &json_dto::mandatory("placename", m_placename) &
        json_dto::mandatory("lat", m_lat) & json_dto::mandatory("lon", m_lon);
  }

  std::string m_placename;
  double      m_lat;
  double      m_lon;
};

// LAB 1
// struct for time and date
struct timeDate_t
{
  timeDate_t() = default;
  timeDate_t(std::string date, std::string time)
      : m_date{std::move(date)}, m_time{std::move(time)}
  {
  }

  template <typename Json_Io> void json_io(Json_Io &io)
  {
    io &json_dto::mandatory("date", m_date) &
        json_dto::mandatory("time", m_time);
  }

  std::string m_date;
  std::string m_time;
};

// LAB 1
// struct for weather station
struct weatherstation_t
{
  weatherstation_t() = default;
  weatherstation_t(int id, timeDate_t dateandtime, place_t place,
                   std::string temperature, std::string humidity)
      : m_id{std::move(id)},
        m_dateTime{std::move(dateandtime)},
        m_place{std::move(place)},
        m_temperature{std::move(temperature)},
        m_humidity{std::move(humidity)}
  {
  }

  template <typename Json_Io> void json_io(Json_Io &io)
  {
    io &json_dto::mandatory("id", m_id) &
        json_dto::mandatory("dateandtime", m_dateTime) &
        json_dto::mandatory("place", m_place) &
        json_dto::mandatory("temperature", m_temperature) &
        json_dto::mandatory("humidity", m_humidity);
  }

  int         m_id;
  timeDate_t  m_dateTime;
  place_t     m_place;
  std::string m_temperature;
  std::string m_humidity;
};

using weather_collection_t = std::vector<weatherstation_t>;

class weather_handler_t
{
public:
  explicit weather_handler_t(weather_collection_t &weather) : m_weather(weather)
  {
  }

  weather_handler_t(const weather_handler_t &) = delete;
  weather_handler_t(weather_handler_t &&)      = delete;

  // LAB 1
  auto on_weather_list(const restinio::request_handle_t &req,
                       rr::route_params_t) const
  {
    auto resp = init_resp(req->create_response());

    resp.set_body(json_dto::to_json(m_weather));

    return resp.done();
  }

  auto on_date_get(const restinio::request_handle_t &req,
                   rr::route_params_t                params)
  {
    int        counter = 0;
    const auto date    = restinio::cast_to<std::uint32_t>(params["date"]);

    auto resp = init_resp(req->create_response());
    for (std::size_t i = 0; i < m_weather.size(); ++i)
    {
      const auto &b = m_weather[i];
      const auto &d = b.m_dateTime;
      if (std::to_string(date) == d.m_date)
      {
        resp.append_body(json_dto::to_json(b));
        counter++;
      }
    }

    if (counter == 0)
    {
      resp.set_body("No data from " + std::to_string(date) + "\n");
    }

    return resp.done();
  }

  auto on_weather_latest(const restinio::request_handle_t &req,
                         rr::route_params_t                params)
  {
    auto resp = init_resp(req->create_response());
    try
    {
      // auto author = restinio::utils::unescape_percent_encoding( params[
      // "author" ] );

      resp.set_body("3 latest entries:\n");

      for (std::size_t i = 0; i < m_weather.size(); ++i)
      {
        const auto &b = m_weather[i];
        if (i >= m_weather.size() - 3)
        {
          resp.append_body(json_dto::to_json(b));
        }
      }
    }
    catch (const std::exception &)
    {
      mark_as_bad_request(resp);
    }

    return resp.done();
  }

  auto on_post_data(const restinio::request_handle_t &req, rr::route_params_t)
  {
    auto resp = init_resp(req->create_response());

    try
    {
      m_weather.emplace_back(
          json_dto::from_json<weatherstation_t>(req->body()));
    }
    catch (const std::exception & /*ex*/)
    {
      mark_as_bad_request(resp);
    }

    // sendMessage("POST: ID=
    // "+json_dto::from_json<weatherstation_t>(req->body()).m_id);
    return resp.done();
  }

  auto on_weather_update(const restinio::request_handle_t &req,
                         rr::route_params_t                params)
  {
    const auto id = restinio::cast_to<std::uint32_t>(params["id"]);

    auto resp = init_resp(req->create_response());

    try
    {
      auto b = json_dto::from_json<weatherstation_t>(req->body());

      if (0 != id && id <= m_weather.size())
      {
        m_weather[id - 1] = b;
      }
      else
      {
        mark_as_bad_request(resp);
        resp.set_body("No data with id " + std::to_string(id) + "\n");
      }
    }
    catch (const std::exception & /*ex*/)
    {
      mark_as_bad_request(resp);
    }

    return resp.done();
  }

  auto on_live_update(const restinio::request_handle_t &req,
                      rr::route_params_t                params)
  {
    if (restinio::http_connection_header_t::upgrade ==
        req->header().connection())
    {
      auto wsh = rws::upgrade<traits_t>(
          *req, rws::activation_t::immediate,
          [this](auto wsh, auto m)
          {
            if (rws::opcode_t::text_frame == m->opcode() ||
                rws::opcode_t::binary_frame == m->opcode() ||
                rws::opcode_t::continuation_frame == m->opcode())
            {
              wsh->send_message(*m);
            }
            else if (rws::opcode_t::ping_frame == m->opcode())
            {
              auto resp = *m;
              resp.set_opcode(rws::opcode_t::pong_frame);
              wsh->send_message(resp);
            }
            else if (rws::opcode_t::connection_close_frame == m->opcode())
            {
              m_registry.erase(wsh->connection_id());
            }
          });
      m_registry.emplace(wsh->connection_id(), wsh);

      init_resp(req->create_response()).done();

      return restinio::request_accepted();
    }

    return restinio::request_rejected();
  }

  auto on_weather_delete(const restinio::request_handle_t &req,
                         rr::route_params_t                params)
  {
    auto resp = init_resp(req->create_response());

    const auto ID = restinio::cast_to<int>(params["id"]);

    try
    {
      for (std::size_t i = 0; i < m_weather.size(); i++)
      {
        const auto &c = m_weather[i];
        if (ID == c.m_id)
        {
          m_weather.erase(m_weather.begin() + i);
        }
      }
    }
    catch (const std::exception & /*ex*/)
    {
      mark_as_bad_request(resp);
    }

    return resp.done();
  }

  auto options(restinio::request_handle_t req, restinio::router::route_params_t)
  {
    const auto methods = "OPTIONS, GET, POST, PUT, PATCH, DELETE, PUT";
    auto       resp    = init_resp(req->create_response());
    resp.append_header(restinio::http_field::access_control_allow_methods,
                       methods);
    resp.append_header(restinio::http_field::access_control_allow_headers,
                       "content-type");
    resp.append_header(restinio::http_field::access_control_max_age, "86400");
    return resp.done();
  }

private:
  weather_collection_t &m_weather;
  ws_registry_t         m_registry;

  // void sendMessage(std::string message)
  // {
  // 	for(auto [k, v] : m_registry)
  // 	{
  // 		v-> send_message(rws::final_frame, rws::opcode_t::text_frame,
  // message);
  // 	}
  // };
 
  template <typename RESP> static RESP init_resp(RESP resp)
  {
    resp.append_header("Server", "RESTinio sample server /v.0.6")
        .append_header_date_field()
        .append_header("Content-Type", "text/plain; charset=utf-8")
        .append_header(restinio::http_field::access_control_allow_origin, "*");

    return resp;
  }

  template <typename RESP> static void mark_as_bad_request(RESP &resp)
  {
    resp.header().status_line(restinio::status_bad_request());
  }
};

auto server_handler(weather_collection_t &weather_collection)
{
  auto router = std::make_unique<router_t>();
  auto handler =
      std::make_shared<weather_handler_t>(std::ref(weather_collection));

  auto by = [&](auto method)
  {
    using namespace std::placeholders;
    return std::bind(method, handler, _1, _2);
  };

  auto method_not_allowed = [](const auto &req, auto)
  {
    return req->create_response(restinio::status_method_not_allowed())
        .connection_close()
        .done();
  };

  // Handlers for '/' path.
  router->http_get("/", by(&weather_handler_t::on_weather_list));
  router->http_post("/", by(&weather_handler_t::on_post_data));
  router->add_handler(restinio::http_method_options(), "/",
                      by(&weather_handler_t::options));

  // Handler for'/latest' path
  router->http_get("/latest", by(&weather_handler_t::on_weather_latest));
  router->add_handler(restinio::http_method_options(), "/latest",
                      by(&weather_handler_t::options));

  //
  router->http_get("/chat", by(&weather_handler_t::on_live_update));

  // Disable all other methods for '/'.
  router->add_handler(
      restinio::router::none_of_methods(restinio::http_method_get(),
                                        restinio::http_method_post(),
                                        restinio::http_method_options()),
      "/", method_not_allowed);

  // Handler for '/date/:date' path.
  router->http_get("/date/:date", by(&weather_handler_t::on_date_get));
  router->add_handler(restinio::http_method_options(), "/date/:date",
                      by(&weather_handler_t::options));

  // Handler for 'R"(/:id(\d+))' path.
  router->http_put(R"(/:id(\d+))", by(&weather_handler_t::on_weather_update));
  router->add_handler(restinio::http_method_options(), R"(/:id(\d+))",
                      by(&weather_handler_t::options));
  // Handler for delete by ID
  router->http_delete(R"(/:id(\d+))",
                      by(&weather_handler_t::on_weather_delete));

  return router;
}

int main()
{
  using namespace std::chrono;

  try
  {
    using traits_t =
        restinio::traits_t<restinio::asio_timer_manager_t,
                           restinio::single_threaded_ostream_logger_t,
                           router_t>;
    // LAB 1
    weather_collection_t weather_collection{{1, timeDate_t{"25042022", "10:25"},
                                             place_t{"Aarhus N", 255.33, 21.3},
                                             "25 deg", "40%"}};

    restinio::run(restinio::on_this_thread<traits_t>()
                      .address("localhost")
                      .request_handler(server_handler(weather_collection))
                      .read_next_http_message_timelimit(10s)
                      .write_http_response_timelimit(1s)
                      .handle_request_timeout(1s));
  }
  catch (const std::exception &ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}