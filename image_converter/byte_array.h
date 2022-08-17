#pragma once

#include <cstdint>
#include <vector>

class byte_array : public std::vector<uint8_t> {
public:
	uint32_t    get_dword_le(size_t pos) { return at(pos) | (at(pos + 1) << 8) | (at(pos + 2) << 16) | (at(pos + 3) << 24); }
	uint16_t    get_word_le(size_t pos)  { return at(pos) | (at(pos + 1) << 8); }
	uint8_t     get_byte_le(size_t pos)  { return at(pos); }

	void set_elastic(size_t pos, uint8_t val) {		// Extend vector if position exceeded the length
		if (size() <= pos) {
			resize(pos + 1);
		}
		at(pos) = val;
	}

	void set_dword_le(size_t pos, uint32_t val) {
		set_elastic(pos    ,  val        & 0xffu);
		set_elastic(pos + 1, (val >>  8) & 0xffu);
		set_elastic(pos + 2, (val >> 16) & 0xffu);
		set_elastic(pos + 3, (val >> 24) & 0xffu);
	}

	void set_word_le(size_t pos, uint32_t val) {
		set_elastic(pos    ,  val        & 0xffu);
		set_elastic(pos + 1, (val >>  8) & 0xffu);
	}
	void set_byte_le(size_t pos, uint32_t val) {
		set_elastic(pos    , val         & 0xffu);
	}
	std::string get_string_z(size_t pos, size_t limit = 256) {
		std::string res;
		size_t count = 0;
		while (at(pos) != '\0' && count++ < limit) {
			res.append(1, at(pos++));
		}
		return res;
	}
	void set_string_z(size_t pos, const std::string str, size_t limit=256) {
		size_t count = 0;
		for (auto it = str.begin(); it != str.end(); ++it) {
			if (count++ >= limit-1) break;		// -1 for '\0'
			set_elastic(pos++, *it);
		}
		set_elastic(pos, '\0');
	}
	void fill(size_t pos, size_t count, uint8_t val) {
		for (size_t wp = pos; wp < pos + count; wp++) {
			set_elastic(wp, val);
		}
	}
	byte_array get_block(size_t pos, size_t count) {
		byte_array res;
		for (size_t i = 0; i < count; i++) {
			res.push_back(at(pos + i));
		}
		return res;
	}
	void set_block(size_t pos, byte_array data) {
		for (auto it = data.begin(); it != data.end(); ++it) {
			at(pos++) = *it;
		}
	}
};
