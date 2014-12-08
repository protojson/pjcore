# pjcore
JSON [de]serialization for protobuf + embedded HTTP server and client in C++.

## Overview
pjcore is a well-tested cross-platform library in C++ for handling [JSON](http://www.json.org/), [Protocol Buffers](https://github.com/google/protobuf/) (protobuf) and HTTP that
* Implements a JSON parser with optional support for trailing commas, comments, signed and unsigned 64-bit integers, NaN and Infinity values,
* Implements a JSON printer with optional support for pretty-print,
* Implements functions for inline JSON construction, so that `MakeJsonObject("alpha", 3, "beta", MakeJsonArray(true, 7.5))` results in {"alpha": 3, "beta": [true, 7.5]},
* Establishes a natural mapping of protobuf message types to JSON, where messages are mapped to objects and repeated fields - to arrays,
* Implements serialization of protobuf messages to JSON format,
* Implements deserialization (parsing) of JSON format into protobuf messages, with support for safe type conversions like int to double and string with decimal value - to int,
* Defines HttpRequest and HttpResponse protobuf messages, encapsulating the most common cases of HTTP requests and responses, respectively,
* Implements an HTTP client that for a given HttpRequest with some URL fetches HttpResponse for that URL,
* Implements an HTTP server that listens to some port and handles requests from connecting clients by invoking your handler that takes HttpRequest and returns HttpResponse,
* Uses loose coupling and eschews singletons so that you can instantiate and use any number of HTTP clients and servers, and when you're done, they can be destroyed without leaking memory, connections or other resources,
* Uses [libuv](https://github.com/libuv/libuv) event loop mechanism for HTTP I/O, so that it can be used together with other libuv-based libraries.

## Build

After you've cloned the project with

```
git clone https://github.com/protojson/pjcore.git
```

be sure to check out the external submodules with

```
git submodule init
git submodule update
```

Then on Linux or Mac run

```
./build.sh
```

or on Windows run

```
build.bat
```

to build the dependencies, library, unit tests and use cases.

## JSON use cases

### You have a protobuf message and want to output it as JSON

```
message Business {
  optional string name = 1;
  optional string url = 2;
}

Business bookstore;
bookstore.set_name("The Shop Around The Corner");
bookstore.set_url("http://bookstore.com");

std::cout << pjcore::WriteJson(bookstore) << std::endl;
```

results in

```
{"name":"The Shop Around The Corner","url":"http:\/\/bookstore.com"}
```

### The same, but more pretty

```
std::cout << pjcore::WritePrettyJson(bookstore) << std::endl;
```

results in

```
{
  "name": "The Shop Around The Corner",
  "url": "http:\/\/bookstore.com"
}
```

### You have a JSON blob and want to parse it as a protobuf

```
message Article {
  optional string title = 1;
  optional string content = 2;
}

pjcore::Error error;
pjcore::JsonValue json;

if (!pjcore::ReadJson("{\"title\":\"Lorem Ipsum\","
                        "\"content\":\"Dolor sit amet.\"}",
                      &json, &error)) {
  // error
} else {
  Article article;
  if (!pjcore::UnboxJsonValue(json, &article, &error)) {
    // error
  } else {
    std::cout << article.DebugString() << std::endl;
  }
}
```

results in

```
title: "Lorem Ipsum"
content: "Dolor sit amet."
```

### JSON blob may have yet unknown properties 

```
message Review {
  optional string reviewer = 1;
  optional double rating = 2;
  repeated pjcore.JsonValue.Property object_properties = 3;
}

pjcore::Error error;
pjcore::JsonValue json;

if (!pjcore::ReadJson("{\"reviewer\":\"Vegeta\","
                        "\"rating\":9001,"
                        "\"summary\":\"It's over nine thousand!\"}",
                      &json, &error)) {
  // error
} else {
  Review review;
  if (!pjcore::UnboxJsonValue(json, &review, &error)) {
    // error
  } else {
    std::cout << review.DebugString() << std::endl;
  }
}
```

results in

```
reviewer: "Vegeta"
rating: 9001
object_properties {
  name: "summary"
  value {
    type: TYPE_STRING
    string_value: "It\'s over nine thousand!"
  }
}
```

## HTTP use cases

### You have a URL and want to fetch its content

```
bool g_client_destroyed = false;

void OnClientDestroy() {
  g_client_destroyed = true;
}

scoped_ptr<HttpResponse> g_response;
Error g_error;

void OnResponse(scoped_ptr<HttpResponse> response,
                const Error& error) {
  g_response = response.Pass();
  g_error = error;
}

Error error;
SharedHttpHandler client = HttpClient::Create(
    HttpClientConfig(), Bind(&OnClientDestroy), &error);

if (!client) {
  // error
} else {
  scoped_ptr<HttpRequest> request(new HttpRequest());
  request->set_url("http://status.aws.amazon.com/rss/ec2-us-west-1.rss");
  
  client->AsyncHandle(request.Pass(), Bind(&OnResponse));
  client = NULL;
  
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  
  if (!g_response) {
    // error
  } else {
    std::cout << g_response->content() << std::endl;
  }
}
```

results in

```
<?xml version="1.0" encoding="UTF-8"?>
<rss version="2.0">
  <channel>
    <title>Amazon Elastic Compute Cloud (N. California) Service Status</title>
    <link>http://status.aws.amazon.com/</link>

...

  </channel>
</rss>
 
```

## You want to run a web server

```
class HelloWorldHttpHandler : public AbstractHttpHandler {
 public:
  HelloWorldHttpHandler()
      : AbstractHttpHandler("HelloWorldHttpHandler", NULL) {}

  void AsyncHandle(scoped_ptr<HttpRequest> request,
                   const HttpResponseCallback& on_response) {
    scoped_ptr<HttpResponse> response(
        new HttpResponse());
    response->set_content("Hello, world!\n");
    on_response.Run(response.Pass(), Error());
  }

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const {
    return scoped_ptr<google::protobuf::Message>();
  }
};

bool g_server_destroyed = false;

void OnServerDestroy() {
  g_server_destroyed = true;
}

HttpServerConfig config;
config.set_port(8080);

SharedHttpHandler handler(new HelloWorldHttpHandler());

Error error;
SharedHttpServer server = HttpServer::Create(
    config, handler, &error);

if (!server) {
  // error
} else if (!server->InitAsync(Bind(&OnServerDestroy), &error)) {
  // error
  server = NULL;
  while (!g_server_destroyed) {
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  }
} else {
  for (;;) {
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  }
}

> curl http://localhost:8080/
```

results in

```
Hello, world!
```

## Copyright, license and dependencies
Unless otherwise stated, the copyright of all pjcore content belongs to the owners of http://protojson.com and is licensed under [GNU General Public License, version 2](http://www.gnu.org/licenses/gpl-2.0.html). Feel free to [contact us](mailto:sales@protojson.com) if you would like to consider a different license agreement for pjcore that would simplify its proprietary/commercial use.

<p>Below is the list of pjcore dependencies with the details of their usage by pjcore, license type and copyright information. 

### Protocol Buffers
[Protocol Buffers](https://github.com/google/protobuf/) (protobuf) are language-neutral, platform-neutral, extensible mechanism for serializing structured data.
<p><b>Usage:</b> pjcore library links against protobuf library, also includes code generated by protobuf compiler for structured data.
<p><b>License:</b> [BSD 3-clause](https://github.com/google/protobuf/blob/master/LICENSE) 
<p><b>Copyright:</b> Google Inc.

### Chromium
[Chromium](http://dev.chromium.org/developers/how-tos/get-the-code) is an open-source browser project that aims to build a safer, faster, and more stable way for all Internet users to experience the web.
<p><b>Usage:</b> pjcore library includes parts of it for callbacks, reference-counted and scoped pointers, lightweight strings and atomic operations.
<p><b>License:</b> [BSD 3-clause](http://src.chromium.org/viewvc/chrome/trunk/src/LICENSE)
<p><b>Copyright:</b> The Chromium Authors

### Dynamic Annotations
[Dynamic Annotations](http://code.google.com/p/data-race-test/wiki/DynamicAnnotations) is a source code annotation that affects the generated code.
<p><b>Usage:</b> pjcore library includes it as a dependency for the used subset of Chromium.
<p><b>License:</b> [BSD 2-clause](https://chromium.googlesource.com/chromium/src/base/+/master/third_party/dynamic_annotations/LICENSE)
<p><b>Copyright:</b> Google Inc.

### Google Test
[Google Test](https://code.google.com/p/googletest/) is a framework for writing C++ tests based on the xUnit architecture.
<p><b>Usage:</b> pjcore unit tests link against it, and also pjcore library includes parts of it for approximate floating-point number comparison.
<p><b>License:</b> [BSD 3-clause](http://opensource.org/licenses/BSD-3-Clause)
<p><b>Copyright:</b> Google Inc.

### Gogle Mock
[Google Mock](https://code.google.com/p/googlemock/) is a library for writing and using C++ mock classes.
<p><b>Usage:</b> pjcore unit tests link against it it.
<p><b>License:</b> [BSD 3-clause](http://opensource.org/licenses/BSD-3-Clause)
<p><b>Copyright:</b> Google Inc.

### Valgrind
[Valgrind](http://valgrind.org/) is an instrumentation framework for building dynamic analysis tools.
<p><b>Usage:</b> pjcore library includes parts of it that are a dependency for the used subset of Chromium.
<p><b>License:</b> [BSD-style license](https://chromium.googlesource.com/chromium/src/base/+/master/third_party/valgrind/LICENSE)
<p><b>Copyright:</b> Julian Seward

### libuv
[libuv](https://github.com/libuv/libuv) is a multi-platform support library with a focus on asynchronous I/O.
<p><b>Usage:</b> pjcore library links against it for asynchronous I/O operations.
<p><b>License:</b> [Node's license](https://github.com/libuv/libuv/blob/master/LICENSE)
<p><b>Copyright:</b> Joyent, Inc. and other Node contributors

### http-parser
[http-parser](https://github.com/joyent/http-parser) is a parser for HTTP messages written in C that parses both requests and responses.
<p><b>Usage:</b> pjcore library links against it for parsing HTTP requests and responses, and also pjcore unit tests include parts of it for testing its own HTTP and URL parsing.
<p><b>License:</b> [MIT](https://github.com/libuv/libuv/blob/master/LICENSE)
<p><b>Copyright:</b> Igor Sysoev, Joyent, Inc. and other Node contributors

### cpplint
[cpplint](http://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py) is a lint-like tool for C++ source code.
<p><b>Usage:</b> cpplint can optionally be used when building pjcore source code to identify style issues.
<p><b>License:</b> [BSD 3-clause](http://google-styleguide.googlecode.com/svn/trunk/cpplint/README)
<p><b>Copyright:</b> Google Inc.

### GYP
[GYP](https://code.google.com/p/gyp/wiki/GypUserDocumentation) is a build automation tool that generates native Visual Studio, Xcode and SCons and/or make build files from a platform-independent input format.
<p><b>Usage:</b> GYP is used for building pjcore dependencies, library, unit tests and tools.
<p><b>License:</b> [BSD 3-clause](https://code.google.com/p/gyp/source/browse/trunk/LICENSE)
<p><b>Copyright:</b> Google Inc.
