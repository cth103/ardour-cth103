#include "canvas/group.h"
#include "canvas/types.h"

namespace ArdourCanvas {

class Text;
class Line;
class Rectangle;

class Flag : public Group
{
public:
	Flag (Group *, Distance, uint32_t, uint32_t, Duple);

	void set_text (std::string const &);
	void set_height (Distance);
	
private:
	Distance _height;
	uint32_t _outline_color;
	uint32_t _fill_color;
	Text* _text;
	Line* _line;
	Rectangle* _rectangle;
};
	
}
