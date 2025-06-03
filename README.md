# 42_webserv

42 Coding School webserver

Summary:
This project is about writing your own HTTP server.
You will be able to test it with an actual browser.
HTTP is one of the most used protocols on the internet.
Knowing its arcane will be useful, even if you won't be working on a website.

---

## Getting Started

Follow these steps to set up and run the webserver:

### 1. Clone the Repository

```
git clone https://github.com/ChimPansky/42_webserv
cd 42_webserv
```

### 2. Compile the Source Files

Make sure you have  `cmake, make` and a C++ compiler installed. Then run:

```
make
```

This will build the project and create the executable in the `build/` directory.

### 3. Configuration File

By default, the server uses the configuration file at `conf/webserv.conf`.
You can edit this file to change server settings, such as ports, server-names, allowed_methods, etc... See `conf/webserv_comments.conf` for available settings

### 4. Run the Webserver

To start the server with the default configuration, run:

```
make run
```

Or, manually:

```
./build/webserv conf/webserv.conf
```

The server will start and listen on the ports specified in the configuration file. You can now open your browser and navigate to the appropriate address (e.g., http://localhost:8080/).

### 5. Accessing from Another Computer

If you want to access the webserver from another computer on the same network, follow these steps:

1. Find the IP address of the host machine by running:

   ```
   ifconfig
   ```

   Look for the `inet` address under your active network interface (e.g., `eth0` or `wlan0`).
2. On the other computer, open a web browser and navigate to:

   ```
   http://<IP>:8080
   ```

   Replace `<IP>` with the IP address you found in step 1.

---

For more details, see the comments in `conf/webserv.conf` and explore the `www/` directory for example content.
