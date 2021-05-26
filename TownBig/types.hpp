#pragma once

union Version
{
	uint64_t full;
	uint16_t sub[4];
	struct
	{
		uint16_t snapshot;
		uint16_t minor;
		uint16_t mid;
		uint16_t major;
	};

	Version(uint16_t snapshot, uint16_t minor, uint16_t mid, uint16_t major)
	{
		this->snapshot = snapshot;
		this->minor = minor;
		this->mid = mid;
		this->major = major;
	}
};

struct WindowData
{
	sf::Vector2u windowedWindowSize;
	bool fullScreen;
};

enum class Inputs : uint8_t
{
	fullScreen, size
};

union TriPoint
{
	GLdouble doubles[8];
	struct
	{
		GLdouble x;
		GLdouble y;
		GLdouble z;
		GLdouble blank0;
		GLdouble texX;
		GLdouble texY;
	};

	TriPoint(GLdouble x, GLdouble y, GLdouble z, GLdouble texX, GLdouble texY)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->texX = texX;
		this->texY = texY;
	}
};