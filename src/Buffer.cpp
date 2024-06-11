#include "Buffer.hpp"

Buffer::Buffer() {
	m_buffer = {"", 0};
}

void Buffer::appendBuffer(const std::string &text) {
	if ((m_buffer.buffer += text) == m_buffer.buffer) return;
	m_buffer.buffer += text;
	m_buffer.length += text.length();
}

void Buffer::freeBuffer() {
	m_buffer = {"", 0};
}

std::string Buffer::getBuffer() {
	return m_buffer.buffer;
}
