Configuration File’s Structure

nginx consists of modules which are controlled by directives specified in the configuration file. Directives are divided into simple directives and block directives. A simple directive consists of the name and parameters separated by spaces and ends with a semicolon (;). A block directive has the same structure as a simple directive, but instead of the semicolon it ends with a set of additional instructions surrounded by braces ({ and }). If a block directive can have other directives inside braces, it is called a context (examples: events, http, server, and location).

Directives placed in the configuration file outside of any contexts are considered to be in the main context. The events and http directives reside in the main context, server in http, and location in server.

HTTP Block
The http context is the top-level context in the nginx.conf file. It contains directives related to HTTP and serves as a container for server blocks.

Server Block
The server context defines the configuration for a specific server or virtual host. It includes directives such as listen (specifying the IP address and port), server_name (defining the domain name), and other server-specific settings.

Location Block
The location context allows you to define configuration rules for specific URL patterns. It is used to specify how NGINX should handle requests for different locations within a server.

Global Directives
The nginx.conf file also contains global directives that apply to the entire NGINX configuration. These directives set global options, define worker processes, and configure logging, among other things.

Worker Process
The number of worker processes in NGINX should ideally match the number of server cores. It can be set to “auto” to automatically utilize all available cores.

Worker Connections
NGINX is designed to handle multiple connections without blocking using a single-threaded worker. The “worker_connections” directive sets the maximum number of connections. In contrast to Apache, which uses one process per connection, NGINX can efficiently handle multiple connections with a single worker.

Dynamic vs. Static Modules
NGINX supports both dynamic and static modules. Static modules are deployed during installation, while dynamic modules, similar to Apache, can be compiled and added to a live server. It’s also possible to compile NGINX with the same Linux OS version on another server or PC.

The rest of a line after the # sign is considered a comment.

Inheritance

In general, a child context – one contained within another context (its parent) – inherits the settings of directives included at the parent level. Some directives can appear in multiple contexts, in which case you can override the setting inherited from the parent by including the directive in the child context. For an example, see the proxy_set_header directive.

https://mohitkr27.medium.com/nginx-configuration-explained-a96d8482d778

https://www.nginxconfig.org/?domains.0.routing.index=index.html&domains.0.routing.fallbackHtml=true&global.nginx.nginxConfigDirectory=.%2F&global.nginx.user=nobody&global.nginx.clientMaxBodySize=100

https://www.plesk.com/blog/various/nginx-configuration-guide/