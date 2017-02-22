#include <eepp/ui/uiprogressbar.hpp>

namespace EE { namespace UI {

UIProgressBar::UIProgressBar( const UIProgressBar::CreateParams& Params ) :
	UIComplexControl( Params ),
	mVerticalExpand( Params.VerticalExpand ),
	mSpeed( Params.MovementSpeed ),
	mFillerMargin( Params.FillerMargin ),
	mDisplayPercent( Params.DisplayPercent ),
	mProgress( 0.f ),
	mTotalSteps( 100.f ),
	mParallax( NULL )
{
	UITextBox::CreateParams TxtBoxParams = Params;

	TxtBoxParams.setParent( this );
	TxtBoxParams.Flags = UI_VALIGN_CENTER | UI_HALIGN_CENTER;
	TxtBoxParams.setPosition( 0, 0 );

	mTextBox = eeNew( UITextBox, ( TxtBoxParams ) );
	mTextBox->setEnabled( false );

	updateTextBox();

	applyDefaultTheme();
}

UIProgressBar::~UIProgressBar() {
	eeSAFE_DELETE( mParallax );
}

Uint32 UIProgressBar::getType() const {
	return UI_TYPE_PROGRESSBAR;
}

bool UIProgressBar::isType( const Uint32& type ) const {
	return UIProgressBar::getType() == type ? true : UIComplexControl::isType( type );
}

void UIProgressBar::draw() {
	UIControlAnim::draw();

	if ( NULL != mParallax && 0.f != mAlpha ) {
		ColorA C( mParallax->getColor() );
		C.Alpha = (Uint8)mAlpha;

		Rectf fillerMargin = dpToPx( mFillerMargin );

		mParallax->setColor( C );
		mParallax->setPosition( Vector2f( mScreenPos.x + fillerMargin.Left, mScreenPos.y + fillerMargin.Top ) );
		mParallax->draw();
	}
}

void UIProgressBar::setTheme( UITheme * Theme ) {
	UIControl::setThemeControl( Theme, "progressbar" );

	if ( mFlags & UI_AUTO_SIZE ) {
		setPixelsSize( mRealSize.x, getSkinSize().getHeight() );
	}

	UISkin * tSkin = Theme->getByName( Theme->getAbbr() + "_progressbar_filler" );

	if ( tSkin ) {
		SubTexture * tSubTexture = tSkin->getSubTexture( UISkinState::StateNormal );

		if ( NULL != tSubTexture ) {
			eeSAFE_DELETE( mParallax );

			Float Height = (Float)getSkinSize().getHeight();

			if ( !mVerticalExpand )
				Height = (Float)tSubTexture->getRealSize().getHeight();

			if ( Height > mRealSize.getHeight() )
				Height = mRealSize.getHeight();

			Rectf fillerMargin = dpToPx( mFillerMargin );

			mParallax = eeNew( ScrollParallax, ( tSubTexture, Vector2f( mScreenPos.x + fillerMargin.Left, mScreenPos.y + fillerMargin.Top ), Sizef( ( ( mRealSize.getWidth() - fillerMargin.Left - fillerMargin.Right ) * mProgress ) / mTotalSteps, Height - fillerMargin.Top - fillerMargin.Bottom ), mSpeed ) );
		}
	}
}

Uint32 UIProgressBar::onValueChange() {
	UIControlAnim::onValueChange();

	onSizeChange();

	return 1;
}

void UIProgressBar::onSizeChange() {
	if ( NULL != mParallax ) {
		Float Height = (Float)mRealSize.getHeight();

		if ( !mVerticalExpand && mParallax->getSubTexture() )
			Height = (Float)mParallax->getSubTexture()->getRealSize().getHeight();

		if ( Height > mRealSize.getHeight() )
			Height = mRealSize.getHeight();

		Rectf fillerMargin = dpToPx( mFillerMargin );

		mParallax->setSize( Sizef( ( ( mRealSize.getWidth() - fillerMargin.Left - fillerMargin.Right ) * mProgress ) / mTotalSteps, Height - fillerMargin.Top - fillerMargin.Bottom ) );
	}

	updateTextBox();
}

void UIProgressBar::setProgress( Float Val ) {
	mProgress = Val;

	onValueChange();
	updateTextBox();
}

const Float& UIProgressBar::getProgress() const {
	return mProgress;
}

void UIProgressBar::setTotalSteps( const Float& Steps ) {
	mTotalSteps = Steps;

	onSizeChange();
	updateTextBox();
}

const Float& UIProgressBar::getTotalSteps() const {
	return mTotalSteps;
}

void UIProgressBar::setMovementSpeed( const Vector2f& Speed ) {
	mSpeed = Speed;

	if ( NULL != mParallax )
		mParallax->setSpeed( mSpeed );
}

const Vector2f& UIProgressBar::getMovementSpeed() const {
	return mSpeed;
}

void UIProgressBar::setVerticalExpand( const bool& VerticalExpand ) {
	if ( VerticalExpand != mVerticalExpand ) {
		mVerticalExpand = VerticalExpand;

		onSizeChange();
	}
}

const bool& UIProgressBar::getVerticalExpand() const {
	return mVerticalExpand;
}

void UIProgressBar::setFillerMargin( const Rectf& margin ) {
	mFillerMargin = margin;

	onPositionChange();
	onSizeChange();
}

const Rectf& UIProgressBar::getFillerMargin() const {
	return mFillerMargin;
}

void UIProgressBar::setDisplayPercent( const bool& DisplayPercent ) {
	mDisplayPercent = DisplayPercent;

	updateTextBox();
}

const bool& UIProgressBar::getDisplayPercent() const {
	return mDisplayPercent;
}

void UIProgressBar::updateTextBox() {
	mTextBox->setVisible( mDisplayPercent );
	mTextBox->setSize( mSize );
	mTextBox->setText( String::toStr( (Int32)( ( mProgress / mTotalSteps ) * 100.f ) ) + "%" );
}

UITextBox * UIProgressBar::getTextBox() const {
	return mTextBox;
}

void UIProgressBar::onAlphaChange() {
	UIControlAnim::onAlphaChange();
	
	mTextBox->setAlpha( mAlpha );
}

}}
