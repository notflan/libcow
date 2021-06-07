#include <cow/area.hpp>

Area::Area(size_t sz) : _area(std::make_unique<Cow>(sz)){}
Area::Area(const Area& copy) : 
	_area(std::make_unique<Cow::Fake>(*copy._area.get())){}
Area::Area(Area&& move) :
	_area(std::move(*const_cast<std::unique_ptr<Cow>*>(&move._area))){}
Area::~Area(){}

Area::Area(Cow&& r) :
	_area(std::make_unique<Cow>(std::move(r))){}
Area::Area(const Cow& r) :
	_area(std::make_unique<Cow::Fake>(r.clone())){}
