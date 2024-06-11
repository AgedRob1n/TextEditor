
#include <string>
class Buffer {
private:
	struct WriteBuffer {std::string buffer; unsigned int length;};
	struct WriteBuffer m_buffer;
public:
	Buffer();
	void appendBuffer(const std::string &text);
	void freeBuffer();
	std::string getBuffer();
};

