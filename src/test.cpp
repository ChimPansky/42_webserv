#include <memory>
#include <map>

int main(int ac, char **av) {
	typedef std::map</* fd */ int, std::auto_ptr<int> > Callbacks;
	Callbacks rd_sock;
    rd_sock.insert(std::make_pair(1, new int(2)));
	Callbacks::iterator it;
	for (it = rd_sock.begin(); it != rd_sock.end(); ++it) {}
	return 0;
}
