#include "cshapesegment.hpp"
#include "cspace.hpp"

CP_NAMESPACE_BEGIN

cShapeSegment * cShapeSegment::New( cBody * body, cVect a, cVect b, cpFloat radius ) {
	return cpNew( cShapeSegment, ( body, a, b, radius ) );
}

cShapeSegment::cShapeSegment( cBody * body, cVect a, cVect b, cpFloat radius ) {
	mShape = cpSegmentShapeNew( body->Body(), tocpv( a ), tocpv( b ), radius );
	SetData();
}

cVect cShapeSegment::A() {
	return tovect( cpSegmentShapeGetA( mShape ) );
}

cVect cShapeSegment::B() {
	return tovect( cpSegmentShapeGetB( mShape ) );
}

cVect cShapeSegment::Normal() {
	return tovect( cpSegmentShapeGetNormal( mShape ) );
}

cpFloat cShapeSegment::Radius() {
	return cpSegmentShapeGetRadius( mShape );
}

void cShapeSegment::Radius( const cpFloat& radius ) {
	cpSegmentShapeSetRadius( mShape, radius );
}

void cShapeSegment::Endpoints( const cVect& a, const cVect& b ) {
	cpSegmentShapeSetEndpoints( mShape, tocpv( a ), tocpv( b ) );
}

bool cShapeSegment::Query( cVect a, cVect b, cpSegmentQueryInfo * info ) {
	return 0 != cpShapeSegmentQuery( mShape, tocpv( a ), tocpv( b ), info );
}

cVect cShapeSegment::QueryHitPoint( const cVect start, const cVect end, const cpSegmentQueryInfo info ) {
	return tovect( cpSegmentQueryHitPoint( tocpv( start ), tocpv( end ), info ) );
}

cpFloat cShapeSegment::QueryHitDist( const cVect start, const cVect end, const cpSegmentQueryInfo info ) {
	return cpSegmentQueryHitDist( tocpv( start ), tocpv( end ), info );
}

void cShapeSegment::Draw( cSpace * space ) {
	//! FIXME: cShapeSegment::Draw
	#ifdef PHYSICS_RENDERER_ENABLED
	cpSegmentShape * seg = (cpSegmentShape *)mShape;
	cVect a = tovect( seg->CP_PRIVATE(ta) );
	cVect b = tovect( seg->CP_PRIVATE(tb) );

	if ( seg->CP_PRIVATE(r) ) {
		GLi->Disable( GL_TEXTURE_2D );
		GLi->DisableClientState( GL_TEXTURE_COORD_ARRAY );
		GLi->DisableClientState( GL_COLOR_ARRAY );

		GLi->VertexPointer( 3, GL_FLOAT, 0, pillVAR, pillVAR_count * sizeof(GLfloat) * 3 );
		GLi->PushMatrix();

		cVect d = b - a;
		cVect r = d * ( seg->CP_PRIVATE(r) / cpvlength( tocpv( d ) ) );

		const GLfloat matrix[] = {
			 r.x, r.y, 0.0f, 0.0f,
			-r.y, r.x, 0.0f, 0.0f,
			 d.x, d.y, 0.0f, 0.0f,
			 a.x, a.y, 0.0f, 1.0f,
		};
		GLi->MultMatrixf(matrix);

		if( !seg->CP_PRIVATE(shape).sensor ){
			eeColorA C = ColorForShape( mShape, space->Space() );

			#ifndef EE_GLES2
			glColor3ub( C.R(), C.B(), C.B() );
			#endif

			GLi->DrawArrays(GL_TRIANGLE_FAN, 0, pillVAR_count);
		}

		#ifndef EE_GLES2
		glColor3f( 0.4f, 0.4f, 0.4f );
		#endif

		GLi->DrawArrays( GL_LINE_LOOP, 0, pillVAR_count );

		GLi->PopMatrix();

		GLi->EnableClientState( GL_TEXTURE_COORD_ARRAY );
		GLi->EnableClientState( GL_COLOR_ARRAY );
		GLi->Enable( GL_TEXTURE_2D );
	} else {
		cPrimitives p;
		p.DrawLine( eeVector2f( a.x, a.y ), eeVector2f( b.x, b.y ) );
	}
	#endif
}

CP_NAMESPACE_END
