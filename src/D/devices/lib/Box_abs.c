Box_abs(x1,y1,x2,y2)
{
	static int x[4], y[4] ;
	if (x2 > x1)
		x2++ ;
	else
		x1++ ;
	x[0] = x1 ; y[0] = y1 ;
	x[1] = x1 ; y[1] = y2 ;
	x[2] = x2 ; y[2] = y2 ;
	x[3] = x2 ; y[3] = y1 ;

	Polygon_abs(x, y, 4) ;
}
