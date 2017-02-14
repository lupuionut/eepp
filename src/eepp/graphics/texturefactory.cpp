#include <eepp/graphics/texturefactory.hpp>
#include <eepp/graphics/textureloader.hpp>
#include <eepp/graphics/glextensions.hpp>
#include <eepp/graphics/renderer/gl.hpp>
#include <eepp/graphics/texture.hpp>
#include <eepp/helper/SOIL2/src/SOIL2/stb_image.h>
#include <eepp/helper/SOIL2/src/SOIL2/SOIL2.h>
#include <eepp/helper/jpeg-compressor/jpge.h>

namespace EE { namespace Graphics {

SINGLETON_DECLARE_IMPLEMENTATION(TextureFactory)

TextureFactory::TextureFactory() :
	mLastBlend(ALPHA_NORMAL),
	mMemSize(0),
	mErasing(false)
{
	mTextures.clear();
	mTextures.push_back( NULL );

	memset( &mCurrentTexture[0], 0, EE_MAX_TEXTURE_UNITS );
}

TextureFactory::~TextureFactory() {
	UnloadTextures();
}

Uint32 TextureFactory::CreateEmptyTexture( const unsigned int& Width, const unsigned int& Height, const unsigned int& Channels, const ColorA& DefaultColor, const bool& Mipmap, const EE_CLAMP_MODE& ClampMode, const bool& CompressTexture, const bool& KeepLocalCopy ) {
	Image TmpImg( Width, Height, Channels, DefaultColor );
	return LoadFromPixels( TmpImg.GetPixelsPtr(), Width, Height, Channels, Mipmap, ClampMode, CompressTexture, KeepLocalCopy );
}

Uint32 TextureFactory::LoadFromPixels( const unsigned char * Pixels, const unsigned int& Width, const unsigned int& Height, const unsigned int& Channels, const bool& Mipmap, const EE_CLAMP_MODE& ClampMode, const bool& CompressTexture, const bool& KeepLocalCopy, const std::string& FileName ) {
	TextureLoader myTex( Pixels, Width, Height, Channels, Mipmap, ClampMode, CompressTexture, KeepLocalCopy, FileName );
	myTex.load();
	return myTex.Id();
}

Uint32 TextureFactory::LoadFromPack( Pack* Pack, const std::string& FilePackPath, const bool& Mipmap, const EE_CLAMP_MODE& ClampMode, const bool& CompressTexture, const bool& KeepLocalCopy  ) {
	TextureLoader myTex( Pack, FilePackPath, Mipmap, ClampMode, CompressTexture, KeepLocalCopy );
	myTex.load();
	return myTex.Id();
}

Uint32 TextureFactory::LoadFromMemory( const unsigned char * ImagePtr, const unsigned int& Size, const bool& Mipmap, const EE_CLAMP_MODE& ClampMode, const bool& CompressTexture, const bool& KeepLocalCopy ) {
	TextureLoader myTex( ImagePtr, Size, Mipmap, ClampMode, CompressTexture, KeepLocalCopy );
	myTex.load();
	return myTex.Id();
}

Uint32 TextureFactory::LoadFromStream( IOStream& Stream, const bool& Mipmap, const EE_CLAMP_MODE& ClampMode, const bool& CompressTexture, const bool& KeepLocalCopy ) {
	TextureLoader myTex( Stream, Mipmap, ClampMode, CompressTexture, KeepLocalCopy );
	myTex.load();
	return myTex.Id();
}

Uint32 TextureFactory::Load( const std::string& Filepath, const bool& Mipmap, const EE_CLAMP_MODE& ClampMode, const bool& CompressTexture, const bool& KeepLocalCopy ) {
	TextureLoader myTex( Filepath, Mipmap, ClampMode, CompressTexture, KeepLocalCopy );
	myTex.load();
	return myTex.Id();
}

Uint32 TextureFactory::PushTexture( const std::string& Filepath, const Uint32& TexId, const unsigned int& Width, const unsigned int& Height, const unsigned int& ImgWidth, const unsigned int& ImgHeight, const bool& Mipmap, const unsigned int& Channels, const EE_CLAMP_MODE& ClampMode, const bool& CompressTexture, const bool& LocalCopy, const Uint32& MemSize ) {
	lock();

	Texture * Tex 		= NULL;
	Uint32 Pos;

	std::string FPath( Filepath );

	FileSystem::filePathRemoveProcessPath( FPath );

	Pos = FindFreeSlot();
	Tex = mTextures[ Pos ] = eeNew( Texture, () );

	Tex->Create( TexId, Width, Height, ImgWidth, ImgHeight, Mipmap, Channels, FPath, ClampMode, CompressTexture, MemSize );
	Tex->Id( Pos );

	if ( LocalCopy ) {
		Tex->Lock();
		Tex->Unlock( true, false );
	}

	mMemSize += MemSize;

	unlock();

	return Pos;
}

Uint32 TextureFactory::FindFreeSlot() {
	if ( mVectorFreeSlots.size() ) {
		Uint32 Pos = mVectorFreeSlots.front();

		mVectorFreeSlots.pop_front();

		return Pos;
	}

	mTextures.push_back( NULL );

	return (Uint32)mTextures.size() - 1;
}

void TextureFactory::Bind( const Texture* Tex, const Uint32& TextureUnit ) {
	if( NULL != Tex && mCurrentTexture[ TextureUnit ] != (Int32)Tex->Handle() ) {
		if ( TextureUnit && GLi->IsExtension( EEGL_ARB_multitexture ) )
			SetActiveTextureUnit( TextureUnit );

		GLi->BindTexture( GL_TEXTURE_2D, Tex->Handle() );

		mCurrentTexture[ TextureUnit ] = Tex->Handle();

		if ( TextureUnit && GLi->IsExtension( EEGL_ARB_multitexture ) )
			SetActiveTextureUnit( 0 );
	}
}

void TextureFactory::Bind( const Uint32& TexId, const Uint32& TextureUnit ) {
	Bind( GetTexture( TexId ), TextureUnit );
}

void TextureFactory::UnloadTextures() {
	mErasing = true;

	for ( Uint32 i = 1; i < mTextures.size(); i++ )
		eeSAFE_DELETE( mTextures[i] );

	mErasing = false;

	mTextures.clear();

	eePRINTL( "Textures Unloaded." );
}

bool TextureFactory::Remove( Uint32 TexId ) {
	Texture * Tex;

	if ( TexId < mTextures.size() && NULL != ( Tex = mTextures[ TexId ] ) ) {
		RemoveReference( mTextures[ TexId ] );

		mErasing = true;
		eeDelete( Tex );
		mErasing = false;

		return true;
	}

	return false;
}

void TextureFactory::RemoveReference( Texture * Tex ) {
	mMemSize -= Tex->MemSize();

	int glTexId = Tex->Handle();

	mTextures[ Tex->Id() ] = NULL;

	for ( Uint32 i = 0; i < EE_MAX_TEXTURE_UNITS; i++ ) {
		if ( mCurrentTexture[ i ] == (Int32)glTexId )
			mCurrentTexture[ i ] = 0;
	}

	mVectorFreeSlots.push_back( Tex->Id() );
}

const bool& TextureFactory::IsErasing() const {
	return mErasing;
}

int TextureFactory::GetCurrentTexture( const Uint32& TextureUnit ) const {
	eeASSERT( TextureUnit < EE_MAX_TEXTURE_UNITS );
	return mCurrentTexture[ TextureUnit ];
}

void TextureFactory::SetCurrentTexture( const int& TexId, const Uint32& TextureUnit ) {
	eeASSERT( TextureUnit < EE_MAX_TEXTURE_UNITS );
	mCurrentTexture[ TextureUnit ] = TexId;
}

void TextureFactory::ReloadAllTextures() {
	for ( Uint32 i = 1; i < mTextures.size(); i++ ) {
		Texture* Tex = GetTexture(i);

		if ( Tex )
			Tex->Reload();
	}

	eePRINTL("Textures Reloaded.");
}

void TextureFactory::GrabTextures() {
	for ( Uint32 i = 1; i < mTextures.size(); i++ ) {
		Texture* Tex = GetTexture(i);

		if ( Tex && !Tex->LocalCopy() ) {
			Tex->Lock();
			Tex->Grabed(true);
		}
	}
}

void TextureFactory::UngrabTextures() {
	for ( Uint32 i = 1; i < mTextures.size(); i++ ) {
		Texture* Tex = GetTexture(i);

		if ( NULL != Tex && Tex->Grabed() ) {
			Tex->Reload();
			Tex->Unlock();
			Tex->Grabed(false);
		}
	}
}

void TextureFactory::SetActiveTextureUnit( const Uint32& Unit ) {
	GLi->ActiveTexture( GL_TEXTURE0 + Unit );
}

void TextureFactory::SetTextureEnv( const EE_TEXTURE_PARAM& Param, const Int32& Val ) {
	#ifndef EE_GLES2
	unsigned int lParam = (unsigned int)GLi->GetTextureParamEnum( Param );

	GLi->TexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );

	if( Param == TEX_PARAM_COLOR_FUNC || Param == TEX_PARAM_ALPHA_FUNC ) {
		GLi->TexEnvi( GL_TEXTURE_ENV, lParam, GLi->GetTextureFuncEnum( (EE_TEXTURE_FUNC)Val ) );
	} else if( Param >= TEX_PARAM_COLOR_SOURCE_0 && Param <= TEX_PARAM_ALPHA_SOURCE_2 ) {
		GLi->TexEnvi( GL_TEXTURE_ENV, lParam, GLi->GetTextureSourceEnum( (EE_TEXTURE_SOURCE)Val ) );
	} else if( Param >= TEX_PARAM_COLOR_OP_0 && Param <= TEX_PARAM_ALPHA_OP_2 ) {
		GLi->TexEnvi( GL_TEXTURE_ENV, lParam, GLi->GetTextureOpEnum( (EE_TEXTURE_OP)Val ) );
	} else {
		GLi->TexEnvi( GL_TEXTURE_ENV, lParam, Val );
	}
	#endif
}

unsigned int TextureFactory::GetValidTextureSize( const unsigned int& Size ) {
	if ( GLi->IsExtension( EEGL_ARB_texture_non_power_of_two ) )
		return Size;
	else
		return Math::NextPowOfTwo(Size);
}

bool TextureFactory::SaveImage( const std::string& filepath, const EE_SAVE_TYPE& Format, const unsigned int& Width, const unsigned int& Height, const unsigned int& Channels, const unsigned char* data ) {
	bool Res;

	if ( SAVE_TYPE_JPG != Format ) {
		Res = 0 != SOIL_save_image ( filepath.c_str(), Format, Width, Height, Channels, data );
	} else {
		jpge::params params;
		params.m_quality = Image::JpegQuality();
		Res = jpge::compress_image_to_jpeg_file( filepath.c_str(), Width, Height, Channels, data, params);
	}

	return Res;
}

bool TextureFactory::TextureIdExists( const Uint32& TexId ) {
	return ( TexId < mTextures.size() && TexId > 0 && NULL != mTextures[ TexId ] );
}

Texture * TextureFactory::GetTexture( const Uint32& TexId ) {
	return mTextures[TexId];
}

void TextureFactory::Allocate( const unsigned int& size ) {
	if ( size > mTextures.size() ) {
		mTextures.resize( size + 1, NULL );

		for ( unsigned int i = 1; i < mTextures.size(); i++ )
			mVectorFreeSlots.push_back( i );
	}
}

Texture * TextureFactory::GetByName( const std::string& Name ) {
	return GetByHash( String::hash( Name ) );
}

Texture * TextureFactory::GetByHash( const Uint32& Hash ) {
	Texture * tTex = NULL;

	for ( Uint32 i = (Uint32)mTextures.size() - 1; i > 0; i-- ) {
		tTex = mTextures[ i ];

		if ( NULL != tTex && tTex->HashName() == Hash )
			return mTextures[ i ];
	}

	return NULL;
}

}}
