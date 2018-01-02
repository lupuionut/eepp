#include <eepp/ui/uieventmouse.hpp>
#include <eepp/ui/uinode.hpp>

namespace EE { namespace UI {

UIEventMouse::UIEventMouse( UINode * Ctrl, const Uint32& EventNum, const Vector2i& Pos, const Uint32& Flags ) :
	UIEvent( Ctrl, EventNum ),
	mPos( Pos ),
	mFlags( Flags )
{
}

UIEventMouse::~UIEventMouse()
{
}

const Vector2i& UIEventMouse::getPosition() const {
	return mPos;
}

const Uint32& UIEventMouse::getFlags() const {
	return mFlags;
}

}}

