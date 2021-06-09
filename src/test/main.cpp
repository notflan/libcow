#include <cow.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <array>

#include <vector>

#include <cow/area.hpp>

using namespace _cow_util;

/// UTF8 multibyte 4.
struct utf8_t {
	const static constexpr size_t MULTIBYTE =4; 
	typedef std::array<char, MULTIBYTE+1> Unicode;
	
	constexpr inline utf8_t() { data[0] = 0; }
		
	template<size_t N>
	constexpr inline utf8_t(const char (&buffer)[N])
	{
		static_assert(N<=MULTIBYTE, "Expected multibyte 4");
		data = {
			buffer[0],
			buffer[1],
			buffer[2],
			buffer[3],
			0,
		};
	}

	constexpr inline utf8_t(char ascii) {
		data[0] = ascii;
		data[1] = 0;
	}

	constexpr inline utf8_t(const char* c) {
		for(size_t i=0;i<MULTIBYTE && (data[i] = c[i]); i++) (void)0;	
	}

	constexpr inline utf8_t(const Unicode& data) : data(data) {}
	
		
	constexpr inline operator const char*() const { return &data[0]; }
	constexpr inline operator char*() { return &data[0]; }

	constexpr inline const char* c_str() const { return &data[0]; }
	constexpr inline char* c_str() { return &data[0]; }

	constexpr inline const char* operator&() const { return c_str(); }
	constexpr inline char* operator&() { return c_str(); }

	Unicode data;
};


namespace Tiling {
	struct Map;

	struct Pixel {
		utf8_t ch;
		struct {
			uint8_t r, g, b, a;
		} c8;
	};

	enum class Direction {
		Up, Right, Down, Left
	};

	// Quad-linked list to upper, right, lower, left segments.
	struct Segment { 
		friend class Map;
		Segment(size_t id) : id(id) {}
		//Segment(Map& owner) : owner(owner){}
		
		~Segment() {
			// Unset pointers of neighbours.
			if(above) above->below = nullptr;
			if(right) right->left = nullptr;
			if(below) below->above = nullptr;
			if(left)  left->right = nullptr;
		}
	private:
		size_t id;

		//TODO
		//Cow raw_tiles;
		//Cow taw_graphics;

		// Links will be NULL if there is no loaded segment in their position.
		// The objects themselves live in Map's segment registry (seg_reg).
		Segment* above;
		Segment* right;
		Segment* below;
		Segment* left;
	};

	struct Map {
		// Remove a registered Segment. Resetting its neighbours' links if needed.
		// The reference will be invalid after calling this function.
		void unregister_segment(Segment& segment)
		{
			if(segment.id<SIZE_MAX) {
				seg_reg.erase(seg_reg.begin()+ segment.id);
			} else throw "dead segment unregistered";
			segment.id = SIZE_MAX;
		}
		// Register a new Segment and return it. The Segment will not be connected to any other.
		Segment& register_segment()
		{
			//seg_reg.push_back(Segment(seg_reg.size()));
			seg_reg.emplace_back(seg_reg.size());
			auto& seg = seg_reg.back();
			return seg;
		}
		// Register a new Segment and then attach it to the referenced one at the position specified by `to`.
		Segment& register_segment(Segment& attach, Direction to)
		{
			auto& seg = register_segment();

			switch(to) {
				case Direction::Up:
					attach.above = &seg; break;
				case Direction::Right:
					attach.right = &seg; break;
				case Direction::Down:
					attach.below = &seg; break;
				case Direction::Left:
					attach.left = &seg; break;
			}
			
			return seg;
		}
	private:
		// All segments live here. 
		// All segments have a lifetime lower than this object.
		std::vector<Segment> seg_reg;
	};
}



template<typename T = unsigned char>
void print_slice(Slice<T> memory)
{
	printf("slice: { %p, %lu (%lu bytes) }\n", memory.area(), memory.size(), memory.size_bytes());

}

void write_fake(Cow& clone, const char* string)
{
	strncpy(clone.area_as<char>(), string, clone.size_as<char>()-1);
}

void read_fake(const Cow& clone)
{
	printf("read_fake: %s\n", clone.area_as<char>());
}

void moving_cow(Cow moved)
{
	auto moved_clone = moved.reinterpret<char>();
	strncpy(&moved_clone, "Ummmm....", moved_clone.size());

	read_fake(moved);
}

int main()
{
	Cow _area(4000);

	Area area = std::move(_area);
	write_fake(area, "Hello???");
	Area area2 = area;
	write_fake(area2, "Hi");
	Area area3 = std::move(area2);
	Area area4 = std::move(area);
	read_fake(area3);
	read_fake(area4);

	printf("Is clone: a1: %d, a2: %d\n", area4.is_clone(), area3.is_clone());

	utf8_t ch = "あ";
	utf8_t ch2('a');
	utf8_t ch3 = ch.c_str();
	utf8_t ch4 = ch3.data;
	utf8_t ch5 = ch4;
	(void)ch5;
	printf("Test: %s, %s, %s\n", (const char*)ch, ch2.c_str(), ch3.c_str());

	Cow real(4096);
	memset(real, 0, real.size_bytes());

	printf("Created real: ");
	print_slice(real);
	print_slice(real.slice_wrap(-20, -10));

	write_fake(real, "Hello world");
	read_fake(real);
	
	Cow::Fake clone = real;
	printf("Fake size: %lu\n", clone.size());
	printf("Fake ptr: %p\n", clone.area());

	read_fake(clone);
	write_fake(clone, "hello fake!");
	read_fake(clone);
	read_fake(real);

	printf("First byte of: real = %x, fake = %x\n", real[0], clone[0]);
	moving_cow(std::move(real)); //moving real is fine
	// <-- real is now dropped. But `clone` still holds Rc to _inner (cow_t).
	printf("First byte of: fake = %x\n", clone[0]);
	read_fake(clone); //clone still functions because of refcount on origin.

	printf("Last error: %d, %s\n", cow_err(), *cow_err_msg(cow_err()));
	Cow should_fail(SIZE_MAX);
	printf("Last error: %d, %s\n", cow_err(), *cow_err_msg(cow_err()));
	Cow::Fake should_fail_clone = should_fail;
	printf("Last error: %d, %s\n", cow_err(), *cow_err_msg(cow_err()));
	return 0;
}
