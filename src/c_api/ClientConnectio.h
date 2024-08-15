
// class ClientConnection {
//   private:
//     ClientConnection();
//     ClientConnection(const ClientConnection&);
//     ClientConnection& operator=(const ClientConnection&);
//   public:
//     ClientConnection(in_addr_t ip, short port) {
//         struct sockaddr_in sa;
//         sa.sin_family = AF_INET;
//         sa.sin_port = htons(port);
//         sa.sin_addr.s_addr = htonl(ip);
//         if (::connect(_sock.sockfd(), (struct sockaddr*)&sa, sizeof(sa)) != 0) {
//             throw "EXCEPTION PLACEHOLDER ClientConnection C'TOR";
//         }
//     }
//     ~ClientConnection() {
//         shutdown(_sock.sockfd(), SHUT_RDWR);
//     }
//   private:
//     Socket _sock;
// };
