Nginx Configuration File Structure
This document outlines the structure of the Nginx configuration file, nginx.conf.

Building Blocks:

Directives: These are instructions that control Nginx's behavior. They fall into two categories:
Simple Directives: These consist of a name, parameters, and a semicolon (;).
Block Directives: Similar to simple directives, but use curly braces ({ }) to enclose additional instructions. These create contexts for configuration.
Contexts: Sections within the configuration file that group related directives. They provide a hierarchical structure. Examples include http, server, and location.
Hierarchical Organization:

Main Context: Directives outside any block belong to the main context.
http Context: This top-level context houses directives related to HTTP functionality and serves as a container for server blocks.
server Context: Defines configuration for a specific server or virtual host. Includes directives like listen (specifying IP address and port) and server_name (defining domain name).
location Context: Allows configuration rules for specific URL patterns. Used to determine how Nginx handles requests for different locations within a server.
Additional Considerations:

Global Directives: The nginx.conf file also contains directives that apply to the entire Nginx configuration, setting global options, defining worker processes, and configuring logging.
Worker Processes & Connections: Nginx uses a single-threaded worker model to handle multiple connections efficiently. The number of worker processes should ideally match the available cores (can be set to "auto"). The worker_connections directive sets a limit on the number of connections each worker can handle.
Modules: Nginx supports both static and dynamic modules. Static modules are incorporated during installation, while dynamic modules can be compiled and added later.
Comments: Lines starting with # are considered comments.
Inheritance: Child contexts inherit settings from their parent context. You can override inherited settings by including the directive in the child context.
Learning More:

NGINX Beginner's Guide: https://docs.nginx.com/
Nginx Configuration Reference: https://www.digitalocean.com/community/tutorials/nginx-location-directive
