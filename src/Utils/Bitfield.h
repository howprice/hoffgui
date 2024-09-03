#pragma once

#include "Core/Helpers.h"

#include <stdint.h>

template <typename EnumType>
class Bitfield
{
public:

	Bitfield()
		: m_bits(0)
	{
	}

	Bitfield(EnumType value)
		: m_bits(0)
	{
		SetBit(value);
	}

	Bitfield(const Bitfield& other)
		: m_bits(other.m_bits)
	{
	}

	Bitfield& operator=(const Bitfield& other)
	{
		m_bits = other.m_bits;
		return *this;
	}

	Bitfield& operator|=(const Bitfield& other)
	{
		m_bits |= other.m_bits;
		return *this;
	}

	Bitfield& operator&=(const Bitfield& other)
	{
		m_bits &= other.m_bits;
		return *this;
	}

	Bitfield& operator^=(const Bitfield& other)
	{
		m_bits ^= other.m_bits;
		return *this;
	}

	Bitfield operator|(const Bitfield& other) const
	{
		Bitfield result(*this);
		result |= other;
		return result;
	}

	Bitfield operator&(const Bitfield& other) const
	{
		Bitfield result(*this);
		result &= other;
		return result;
	}

	Bitfield operator^(const Bitfield& other) const
	{
		Bitfield result(*this);
		result ^= other;
		return result;
	}

	bool operator==(const Bitfield& other) const
	{
		return m_bits == other.m_bits;
	}

	bool operator!=(const Bitfield& other) const
	{
		return m_bits != other.m_bits;
	}

	bool operator==(EnumType value) const
	{
		return m_bits == (1 << static_cast<uint32_t>(value));
	}

	bool operator!=(EnumType value) const
	{
		return m_bits != (1 << static_cast<uint32_t>(value));
	}

	bool operator[](EnumType value) const
	{
		return m_bits & (1 << static_cast<uint32_t>(value));
	}

	Bitfield& SetBit(EnumType value)
	{
		m_bits |= 1 << static_cast<uint32_t>(value);
		return *this;
	}

	Bitfield& ClearBit(EnumType value)
	{
		m_bits &= ~(1 << static_cast<uint32_t>(value));
		return *this;
	}

	Bitfield& ToggleBit(EnumType value)
	{
		m_bits ^= 1 << static_cast<uint32_t>(value);
		return *this;
	}

	Bitfield& Clear()
	{
		m_bits = 0;
		return *this;
	}

	bool IsBitSet(EnumType value) const
	{
		return m_bits & (1 << static_cast<uint32_t>(value));
	}

	bool IsBitClear(EnumType value) const
	{
		return !IsBitSet(value);
	}

	bool IsEmpty() const
	{
		return m_bits == 0;
	}

	uint32_t GetBits() const
	{
		return m_bits;
	}

private:

	// #TODO: Templetise storage type iff required e.g. to store 8-bit bitfield
	static_assert(ENUM_COUNT(EnumType) <= sizeof(uint32_t) * 8, "EnumType is too large for Bitfield");
	uint32_t m_bits;
};
