#ifndef ETERM_TERMINALDISPLAY_HPP
#define ETERM_TERMINALDISPLAY_HPP

#include <eepp/config.hpp>
#include <eepp/graphics/font.hpp>
#include <eepp/graphics/framebuffer.hpp>
#include <eepp/system/clock.hpp>
#include <eepp/window/inputevent.hpp>
#include <eepp/window/keycodes.hpp>
#include <eepp/window/window.hpp>
#include <eterm/system/iprocessfactory.hpp>
#include <eterm/terminal/iterminaldisplay.hpp>
#include <eterm/terminal/terminalemulator.hpp>

#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>

using namespace EE;
using namespace EE::Window;
using namespace EE::System;
using namespace eterm::System;
using namespace eterm::Terminal;

namespace eterm { namespace Terminal {

enum class TerminalShortcutAction {
	PASTE,
	COPY,
	SCROLLUP_ROW,
	SCROLLDOWN_ROW,
	SCROLLUP_SCREEN,
	SCROLLDOWN_SCREEN,
	SCROLLUP_HISTORY,
	SCROLLDOWN_HISTORY,
	FONTSIZE_GROW,
	FONTSIZE_SHRINK
};

struct TerminalKey {
	Keycode keysym;
	Uint32 mask;
	const char* string;
	int appkey;
	int appcursor;
};

struct TerminalScancode {
	Scancode scancode;
	Uint32 mask;
	std::string string;
	int appkey;
	int appcursor;
};

struct TerminalShortcut {
	Keycode keysym;
	Uint32 mask;
	TerminalShortcutAction action;
	int appkey;
	int appcursor;
	int altscrn{ 0 }; /* 0: don't care, -1: not alt screen, 1: alt screen */
};

struct TerminalMouseShortcut {
	MouseButtonsMask button;
	Uint32 mask;
	TerminalShortcutAction action;
	int appkey;
	int appcursor;
	int altscrn{ 0 }; /* 0: don't care, -1: not alt screen, 1: alt screen */
};

struct TerminalKeyMapEntry {
	Uint32 mask;
	std::string string;
	int appkey;
	int appcursor;
};

struct TerminalKeyMapShortcut {
	Uint32 mask;
	TerminalShortcutAction action;
	int appkey;
	int appcursor;
	int altscrn{ 0 }; /* 0: don't care, -1: not alt screen, 1: alt screen */
};

class TerminalKeyMap {
  private:
	std::unordered_map<Keycode, std::vector<TerminalKeyMapEntry>> mKeyMap;
	std::unordered_map<Scancode, std::vector<TerminalKeyMapEntry>> mPlatformKeyMap;
	std::unordered_map<Keycode, std::vector<TerminalKeyMapShortcut>> mShortcuts;
	std::unordered_map<Uint32, std::vector<TerminalKeyMapShortcut>> mMouseShortcuts;

  public:
	TerminalKeyMap( const TerminalKey keys[], size_t keysLen, const TerminalScancode platformKeys[],
					size_t platformKeysLen, const TerminalShortcut shortcuts[], size_t shortcutsLen,
					const TerminalMouseShortcut mouseShortcuts[], size_t mouseShortcutsLen );

	inline const std::unordered_map<Keycode, std::vector<TerminalKeyMapEntry>>& KeyMap() const {
		return mKeyMap;
	}

	inline const std::unordered_map<Scancode, std::vector<TerminalKeyMapEntry>>&
	PlatformKeyMap() const {
		return mPlatformKeyMap;
	}

	inline const std::unordered_map<Keycode, std::vector<TerminalKeyMapShortcut>>&
	Shortcuts() const {
		return mShortcuts;
	}

	inline const std::unordered_map<Uint32, std::vector<TerminalKeyMapShortcut>>&
	MouseShortcuts() const {
		return mMouseShortcuts;
	}
};

constexpr int TerminalKeyModFlags_Any = 0xFFFFFFFF;

extern TerminalKeyMap terminalKeyMap;

static const Scancode asciiScancodeTable[] = {
	SCANCODE_A, SCANCODE_B, SCANCODE_C,			  SCANCODE_D,	  SCANCODE_E,			SCANCODE_F,
	SCANCODE_G, SCANCODE_H, SCANCODE_I,			  SCANCODE_J,	  SCANCODE_K,			SCANCODE_L,
	SCANCODE_M, SCANCODE_N, SCANCODE_O,			  SCANCODE_P,	  SCANCODE_Q,			SCANCODE_R,
	SCANCODE_S, SCANCODE_T, SCANCODE_U,			  SCANCODE_V,	  SCANCODE_W,			SCANCODE_X,
	SCANCODE_Y, SCANCODE_Z, SCANCODE_LEFTBRACKET, SCANCODE_SLASH, SCANCODE_RIGHTBRACKET };

class TerminalDisplay : public ITerminalDisplay {
  public:
	enum class EventType { TITLE, ICON_TITLE, SCROLL_HISTORY, HISTORY_LENGTH_CHANGE, UNKNOWN };

	struct Event {
		EventType type{ EventType::UNKNOWN };
		std::string eventData{};
	};

	typedef std::function<void( const TerminalDisplay::Event& event )> EventFunc;

	static std::shared_ptr<TerminalDisplay>
	create( EE::Window::Window* window, Font* font, const Float& fontSize, const Sizef& pixelsSize,
			std::shared_ptr<TerminalEmulator>&& terminalEmulator,
			const bool& useFrameBuffer = false );

	static std::shared_ptr<TerminalDisplay>
	create( EE::Window::Window* window, Font* font, const Float& fontSize, const Sizef& pixelsSize,
			std::string program = "", const std::vector<std::string>& args = {},
			const std::string& workingDir = "", const size_t& historySize = 10000,
			IProcessFactory* processFactory = nullptr, const bool& useFrameBuffer = false );

	virtual ~TerminalDisplay();

	virtual void resetColors();
	virtual int resetColor( Uint32 index, const char* name );

	virtual void setTitle( const char* title );
	virtual void setIconTitle( const char* title );

	virtual void setClipboard( const char* text );
	virtual const char* getClipboard() const;

	virtual bool drawBegin( int columns, int rows );
	virtual void drawLine( Line line, int x1, int y, int x2 );
	virtual void drawCursor( int cx, int cy, TerminalGlyph g, int ox, int oy, TerminalGlyph og );
	virtual void drawEnd();

	virtual void update();

	void action( TerminalShortcutAction action );

	bool hasTerminated() const;

	void draw();

	virtual void onMouseDoubleClick( const Vector2i& pos, const Uint32& flags );

	virtual void onMouseMove( const Vector2i& pos, const Uint32& flags );

	virtual void onMouseDown( const Vector2i& pos, const Uint32& flags );

	virtual void onMouseUp( const Vector2i& pos, const Uint32& flags );

	virtual void onTextInput( const Uint32& chr );

	virtual void onKeyDown( const Keycode& keyCode, const Uint32& chr, const Uint32& mod,
							const Scancode& scancode );

	Font* getFont() const;

	void setFont( Font* font );

	const Float& getFontSize() const;

	void setFontSize( const Float& FontSize );

	const Vector2f& getPosition() const;

	void setPosition( const Vector2f& position );

	const Sizef& getSize() const;

	void setSize( const Sizef& size );

	bool isDirty() const { return mDirty; }

	void invalidate();

	void invalidateCursor();

	void invalidateLine( const int& line );

	void invalidateLines();

	bool hasFocus() const { return mFocus; }

	void setFocus( bool focus );

	bool isBlinkingCursor();

	const Rectf& getPadding() const;

	void setPadding( const Rectf& padding );

	const std::shared_ptr<TerminalEmulator>& getTerminal() const;

	virtual void attach( TerminalEmulator* terminal );

	int scrollSize() const;

	int rowCount() const;

	Uint32 pushEventCallback( const EventFunc& func );

	void popEventCallback( const Uint32& id );

	Float getLineHeight() const;

  protected:
	EE::Window::Window* mWindow;
	std::vector<TerminalGlyph> mBuffer;
	std::vector<std::pair<Color, std::string>> mColors;
	std::shared_ptr<TerminalEmulator> mTerminal;
	mutable String mClipboard;
	mutable std::string mClipboardUtf8;
	Uint32 mNumCallBacks;
	std::map<Uint32, EventFunc> mCallbacks;

	Font* mFont{ nullptr };
	Float mFontSize{ 12 };
	Rectf mPadding;
	Vector2f mPosition;
	Sizef mSize;
	std::vector<bool> mDirtyLines;
	bool mDirty{ true };
	bool mDirtyCursor{ true };
	bool mDrawing{ false };
	Vector2i mCursor;
	TerminalGlyph mCursorGlyph;
	bool mUseColorEmoji{ true };
	bool mPasteNewlineFix{ true };
	bool mFocus{ true };
	bool mUseFrameBuffer{ true };
	bool mAlreadyClickedMButton{ false };
	Clock mClock;
	Clock mLastDoubleClick;
	int mColumns{ 0 };
	int mRows{ 0 };
	FrameBuffer* mFrameBuffer{ nullptr };

	std::string mProgram;
	std::vector<std::string> mArgs;
	std::string mWorkingDir;

	TerminalDisplay( EE::Window::Window* window, Font* font, const Float& fontSize,
					 const Sizef& pixelsSize, const bool& useFrameBuffer );

	void draw( const Vector2f& pos );

	void drawGrid( const Vector2f& pos );

	Vector2i positionToGrid( const Vector2i& pos );

	void onSizeChange();

	virtual void onProcessExit( int exitCode );

	void sendEvent( const TerminalDisplay::Event& event );

	Sizei getFrameBufferSize();

	void createFrameBuffer();

	void drawFrameBuffer();
};

}} // namespace eterm::Terminal

#endif // ETERM_TERMINALDISPLAY_HPP
