#include <eepp/system/log.hpp>
#include <eepp/ui/doc/syntaxdefinitionmanager.hpp>
#include <eepp/ui/doc/syntaxhighlighter.hpp>
#include <eepp/ui/doc/syntaxtokenizer.hpp>

namespace EE { namespace UI { namespace Doc {

Uint64 TokenizedLine::calcSignature( const std::vector<SyntaxTokenPosition>& tokens ) {
	if ( !tokens.empty() ) {
		return String::hash( reinterpret_cast<const char*>( tokens.data() ),
							 sizeof( SyntaxTokenPosition ) * tokens.size() );
	}
	return 0;
}

void TokenizedLine::updateSignature() {
	this->signature = calcSignature( tokens );
}

SyntaxHighlighter::SyntaxHighlighter( TextDocument* doc ) :
	mDoc( doc ), mFirstInvalidLine( 0 ), mMaxWantedLine( 0 ) {
	reset();
}

void SyntaxHighlighter::changeDoc( TextDocument* doc ) {
	mDoc = doc;
	reset();
	mMaxWantedLine = (Int64)mDoc->linesCount() - 1;
}

void SyntaxHighlighter::reset() {
	Lock l( mLinesMutex );
	mLines.clear();
	mFirstInvalidLine = 0;
	mMaxWantedLine = 0;
}

void SyntaxHighlighter::invalidate( Int64 lineIndex ) {
	mFirstInvalidLine = eemin( lineIndex, mFirstInvalidLine );
	mMaxWantedLine = eemin<Int64>( mMaxWantedLine, (Int64)mDoc->linesCount() - 1 );
}

TokenizedLine SyntaxHighlighter::tokenizeLine( const size_t& line, const Uint64& state ) {
	auto& ln = mDoc->line( line );
	TokenizedLine tokenizedLine;
	tokenizedLine.initState = state;
	tokenizedLine.hash = ln.getHash();
	if ( mMaxTokenizationLength != 0 && (Int64)ln.size() > mMaxTokenizationLength ) {
		Int64 textSize = ln.size();
		SyntaxTokenLen pos = 0;
		while ( textSize > 0 ) {
			SyntaxTokenLen chunkSize =
				textSize > mMaxTokenizationLength ? mMaxTokenizationLength : textSize;
			SyntaxTokenPosition token{ SyntaxStyleTypes::Normal, pos, chunkSize };
			token.len = ln.size();
			tokenizedLine.tokens.emplace_back( token );
			textSize -= chunkSize;
			pos += chunkSize;
		}
		tokenizedLine.state = state;
		tokenizedLine.updateSignature();
		return tokenizedLine;
	}
	auto res = SyntaxTokenizer::tokenizePosition( mDoc->getSyntaxDefinition(), ln.toUtf8(), state );
	tokenizedLine.tokens = std::move( res.first );
	tokenizedLine.state = std::move( res.second );
	tokenizedLine.updateSignature();
	return tokenizedLine;
}

Mutex& SyntaxHighlighter::getLinesMutex() {
	return mLinesMutex;
}

void SyntaxHighlighter::moveHighlight( const Int64& fromLine, const Int64& numLines ) {
	if ( mLines.find( fromLine ) == mLines.end() )
		return;
	Int64 linesCount = mDoc->linesCount();
	if ( numLines > 0 ) {
		for ( Int64 i = linesCount - 1; i >= fromLine; --i ) {
			auto lineIt = mLines.find( i - numLines );
			if ( lineIt != mLines.end() ) {
				const auto& line = lineIt->second;
				if ( line.hash == mDoc->line( i ).getHash() ) {
					auto nl = mLines.extract( lineIt );
					nl.key() = i;
					mLines.insert( std::move( nl ) );
				}
			}
		}
	} else if ( numLines < 0 ) {
		for ( Int64 i = fromLine; i < linesCount; i++ ) {
			auto lineIt = mLines.find( i - numLines );
			if ( lineIt != mLines.end() && lineIt->second.hash == mDoc->line( i ).getHash() ) {
				auto nl = mLines.extract( lineIt );
				nl.key() = i;
				mLines[i] = std::move( nl.mapped() );
			}
		}
	}
}

Uint64 SyntaxHighlighter::getTokenizedLineSignature( const size_t& index ) {
	Lock l( mLinesMutex );
	auto line = mLines.find( index );
	if ( line != mLines.end() )
		return line->second.signature;
	return 0;
}

const Int64& SyntaxHighlighter::getMaxTokenizationLength() const {
	return mMaxTokenizationLength;
}

void SyntaxHighlighter::setMaxTokenizationLength( const Int64& maxTokenizationLength ) {
	mMaxTokenizationLength = maxTokenizationLength;
}

const std::vector<SyntaxTokenPosition>& SyntaxHighlighter::getLine( const size_t& index ) {
	if ( mDoc->getSyntaxDefinition().getPatterns().empty() ) {
		static std::vector<SyntaxTokenPosition> noHighlightVector = {
			{ SyntaxStyleTypes::Normal, 0, 0 } };
		noHighlightVector[0].len = mDoc->line( index ).size();
		return noHighlightVector;
	}
	Lock l( mLinesMutex );
	const auto& it = mLines.find( index );
	if ( it == mLines.end() ||
		 ( index < mDoc->linesCount() && mDoc->line( index ).getHash() != it->second.hash ) ) {
		int prevState = SYNTAX_TOKENIZER_STATE_NONE;
		if ( index > 0 ) {
			auto prevIt = mLines.find( index - 1 );
			if ( prevIt != mLines.end() ) {
				prevState = prevIt->second.state;
			}
		}
		mLines[index] = tokenizeLine( index, prevState );
		mTokenizerLines[index] = mLines[index];
		mMaxWantedLine = eemax<Int64>( mMaxWantedLine, index );
		return mLines[index].tokens;
	}
	mMaxWantedLine = eemax<Int64>( mMaxWantedLine, index );
	return it->second.tokens;
}

Int64 SyntaxHighlighter::getFirstInvalidLine() const {
	return mFirstInvalidLine;
}

Int64 SyntaxHighlighter::getMaxWantedLine() const {
	return mMaxWantedLine;
}

bool SyntaxHighlighter::updateDirty( int visibleLinesCount ) {
	if ( visibleLinesCount <= 0 )
		return 0;
	if ( mFirstInvalidLine > mMaxWantedLine ) {
		mMaxWantedLine = 0;
	} else {
		bool changed = false;
		Int64 max = eemax( 0LL, eemin( mFirstInvalidLine + visibleLinesCount, mMaxWantedLine ) );

		for ( Int64 index = mFirstInvalidLine; index <= max; index++ ) {
			Uint64 state = SYNTAX_TOKENIZER_STATE_NONE;
			if ( index > 0 ) {
				auto prevIt = mLines.find( index - 1 );
				if ( prevIt != mLines.end() ) {
					state = prevIt->second.state;
				}
			}
			const auto& it = mLines.find( index );
			if ( it == mLines.end() || it->second.hash != mDoc->line( index ).getHash() ||
				 it->second.initState != state ) {
				mLines[index] = tokenizeLine( index, state );
				mTokenizerLines[index] = mLines[index];
				changed = true;
			}
		}

		mFirstInvalidLine = max + 1;
		return changed;
	}
	return false;
}

const SyntaxDefinition&
SyntaxHighlighter::getSyntaxDefinitionFromTextPosition( const TextPosition& position ) {
	Lock l( mLinesMutex );
	auto found = mLines.find( position.line() );
	if ( found == mLines.end() )
		return SyntaxDefinitionManager::instance()->getPlainStyle();

	TokenizedLine& line = found->second;
	SyntaxState state =
		SyntaxTokenizer::retrieveSyntaxState( mDoc->getSyntaxDefinition(), line.state );

	if ( nullptr == state.currentSyntax )
		return SyntaxDefinitionManager::instance()->getPlainStyle();

	return *state.currentSyntax;
}

SyntaxStyleType SyntaxHighlighter::getTokenTypeAt( const TextPosition& pos ) {
	if ( !pos.isValid() || pos.line() < 0 || pos.line() >= (Int64)mDoc->linesCount() )
		return SyntaxStyleTypes::Normal;
	auto tokens = getLine( pos.line() );
	if ( tokens.empty() )
		return SyntaxStyleTypes::Normal;
	Int64 col = 0;
	for ( const auto& token : tokens ) {
		col += token.len;
		if ( col > pos.column() )
			return token.type;
	}
	return SyntaxStyleTypes::Normal;
}

SyntaxTokenPosition SyntaxHighlighter::getTokenPositionAt( const TextPosition& pos ) {
	if ( !pos.isValid() || pos.line() < 0 || pos.line() >= (Int64)mDoc->linesCount() )
		return { SyntaxStyleTypes::Normal, 0, 0 };
	auto tokens = getLine( pos.line() );
	if ( tokens.empty() )
		return { SyntaxStyleTypes::Normal, 0, 0 };
	Int64 col = 0;
	for ( const auto& token : tokens ) {
		col += token.len;
		if ( col > pos.column() )
			return { token.type, static_cast<SyntaxStyleType>( col - token.len ), token.len };
	}
	return { SyntaxStyleTypes::Normal, 0, 0 };
}

void SyntaxHighlighter::setLine( const size_t& line, const TokenizedLine& tokenization ) {
	Lock l( mLinesMutex );
	mLines[line] = tokenization;
}

void SyntaxHighlighter::mergeLine( const size_t& line, const TokenizedLine& tokenization ) {
	TokenizedLine tline;
	{
		Lock l( mLinesMutex );
		auto found = mTokenizerLines.find( line );
		if ( found != mTokenizerLines.end() &&
			 mDoc->line( line ).getHash() == found->second.hash ) {
			tline = found->second;
		} else {
			tline = tokenizeLine( line );
			mTokenizerLines[line] = tline;
		}
	}

	size_t lastTokenPos = 0;
	for ( const auto& token : tokenization.tokens ) {
		for ( size_t i = lastTokenPos; i < tline.tokens.size(); ++i ) {
			const auto ltoken = tline.tokens[i];
			if ( token.pos >= ltoken.pos && token.pos + token.len <= ltoken.pos + ltoken.len ) {
				tline.tokens.erase( tline.tokens.begin() + i );
				int iDiff = i;

				if ( token.pos > ltoken.pos ) {
					++iDiff;
					tline.tokens.insert(
						tline.tokens.begin() + i,
						{ ltoken.type, ltoken.pos,
						  static_cast<SyntaxTokenLen>( token.pos - ltoken.pos ) } );
				}

				tline.tokens.insert( tline.tokens.begin() + iDiff, token );

				if ( token.pos + token.len < ltoken.pos + ltoken.len ) {
					tline.tokens.insert(
						tline.tokens.begin() + iDiff + 1,
						{ ltoken.type, static_cast<SyntaxTokenLen>( token.pos + token.len ),
						  static_cast<SyntaxTokenLen>( ( ltoken.pos + ltoken.len ) -
													   ( token.pos + token.len ) ) } );
				}

				lastTokenPos = i;
				break;
			}
		}
	}

	tline.signature = tokenization.signature;
	Lock l( mLinesMutex );
	mLines[line] = std::move( tline );
}

}}} // namespace EE::UI::Doc
