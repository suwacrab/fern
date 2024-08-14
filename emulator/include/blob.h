#ifndef BLOB_H
#define BLOB_H

#include <vector>
#include <cstdint>
#include <string>

class Blob {
	private:
		std::vector<uint8_t> m_data;
	public:
		Blob();
		Blob(const Blob& other);
		Blob(const std::string& source_str);

		auto load_file(const std::string& filename, bool allow_fail=false) -> bool;
		auto write_file(const std::string& filename, bool allow_fail=false) -> bool;
		
		auto write_raw(const void* source, size_t size) -> void;
		auto write_u8(const uint32_t num) -> void;
		auto write_u16(const uint32_t num) -> void;
		auto write_u32(const uint32_t num) -> void;
		auto write_blob(const Blob& other) -> void;
		auto write_str(const std::string& source_str) -> void;
		template<typename T> auto write_block(const T* source) {
			write_raw(source,sizeof(T));
		}

		auto align(size_t alignment, uint8_t filler = 0xFF) -> void;

		auto size() const -> size_t { return m_data.size(); }
		template<typename T=uint8_t> auto data() const -> const T* {
			return static_cast<const T*>(m_data.data());
		}

};

#endif

