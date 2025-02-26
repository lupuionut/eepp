#ifndef ECODE_GLOBALSEARCHCONTROLLER_HPP
#define ECODE_GLOBALSEARCHCONTROLLER_HPP

#include "appconfig.hpp"
#include "plugins/pluginmanager.hpp"
#include "projectsearch.hpp"
#include <eepp/ee.hpp>

namespace ecode {

class UIGlobalSearchBar;
class UITreeViewGlobalSearch;
class App;

class GlobalSearchController {
  public:
	static std::unordered_map<std::string, std::string> getDefaultKeybindings() {
		return {
			{ "escape", "close-global-searchbar" },
			{ "mod+s", "change-case" },
			{ "mod+w", "change-whole-word" },
			{ "mod+l", "toggle-lua-pattern" },
			{ "mod+r", "search-replace-in-files" },
			{ "mod+g", "search-again" },
			{ "mod+a", "expand-all" },
			{ "mod+shift+e", "collapse-all" },
			{ "mod+e", "change-escape-sequence" },
			{ "mod+h", "global-search-clear-history" },
		};
	}

	GlobalSearchController( UICodeEditorSplitter*, UISceneNode*, App* );

	void updateGlobalSearchBar();

	void initGlobalSearchBar(
		UIGlobalSearchBar* globalSearchBar, const GlobalSearchBarConfig& globalSearchBarConfig,
		std::unordered_map<std::string, std::string> keybindings = getDefaultKeybindings() );

	void hideGlobalSearchBar();

	void toggleGlobalSearchBar();

	void updateGlobalSearchBarResults( const std::string& search,
									   std::shared_ptr<ProjectSearch::ResultModel> model,
									   bool searchReplace, bool isEscaped );

	void initGlobalSearchTree( UITreeViewGlobalSearch* searchTree );

	void doGlobalSearch( String text, bool caseSensitive, bool wholeWord, bool luaPattern,
						 bool escapeSequence, bool searchReplace, bool searchAgain = false );

	size_t replaceInFiles( const std::string& replaceText,
						   std::shared_ptr<ProjectSearch::ResultModel> model );

	void showGlobalSearch();

	void showGlobalSearch( bool searchAndReplace );

	void updateColorScheme( const SyntaxColorScheme& colorScheme );

	bool isUsingSearchReplaceTree();

	void clearHistory();

	GlobalSearchBarConfig getGlobalSearchBarConfig() const;

  protected:
	UICodeEditorSplitter* mSplitter{ nullptr };
	UISceneNode* mUISceneNode{ nullptr };
	App* mApp{ nullptr };

	UIGlobalSearchBar* mGlobalSearchBarLayout{ nullptr };
	UILayout* mGlobalSearchLayout{ nullptr };
	UITreeViewGlobalSearch* mGlobalSearchTree{ nullptr };
	UITreeViewGlobalSearch* mGlobalSearchTreeSearch{ nullptr };
	UITreeViewGlobalSearch* mGlobalSearchTreeReplace{ nullptr };
	UITextInput* mGlobalSearchInput{ nullptr };
	UIDropDownList* mGlobalSearchHistoryList{ nullptr };
	Uint32 mGlobalSearchHistoryOnItemSelectedCb{ 0 };
	std::deque<std::pair<std::string, std::shared_ptr<ProjectSearch::ResultModel>>>
		mGlobalSearchHistory;

	void onLoadDone( const Variant& lineNum, const Variant& colNum );

	PluginRequestHandle processMessage( const PluginMessage& msg );

	void updateGlobalSearchHistory( std::shared_ptr<ProjectSearch::ResultModel> model,
									const std::string& search, bool searchReplace, bool searchAgain,
									bool escapeSequence );
};

} // namespace ecode

#endif // ECODE_GLOBALSEARCHCONTROLLER_HPP
