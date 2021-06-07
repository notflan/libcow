// `Area` is a copyable type that opaquely represents either `Cow` or `Cow::Fake`.
#pragma once

#include <memory>
#include <utility>

#include <cow.hpp>

namespace _cow_util {
	struct Area {
		Area() = delete;
		
		explicit Area(size_t sz);
		Area(const Area& area);
		Area(Area&& area);

		Area(Cow&& move);
		explicit Area(const Cow& copy);

		inline const Cow* operator->() const { return _area.get(); }
		inline Cow* operator->() { return _area.get(); }

		inline const Cow* operator*() const { return _area.get(); }
		inline Cow* operator*() { return _area.get(); }

		inline operator const Cow&() const { return *_area.get(); }
		inline operator Cow&() { return *_area.get(); }

		inline bool is_clone() const { return dynamic_cast<Cow::Fake*>(_area.get()) != nullptr; }

		inline cow_t* raw() const { return _area->raw(); }

		~Area();
		private:
		const std::unique_ptr<Cow> _area;
	};
}
