// Distance.h.hlsl
// Compute distance between a 2D segment and point.


// distance between segment vw and point p
float distance(float2 v, float2 w, float2 p)
{
	float1 dist = 0;

	// squared length of line segment
	float1 lsq = (v.x-w.x)*(v.x-w.x) + (v.y-w.y)*(v.y-w.y);

	// v and w are the same point
	if (lsq == 0.0)
	{
		dist = length(p - v);
	}
	else
	{
		// project point onto line segment
		float1 t = dot(p - v, w - v) / lsq;

		// point near v
		if (t < 0.0) dist = length(p - v);

		// point near w
		else if (t > 1.0) dist = length(p - w);

		// distance between p and projection point
		else dist = length(p - (v + t * (w - v)));
	}

	return dist;
}


// distance between segment vw and point p, returns projection distance t
float distance(float2 v, float2 w, float2 p, out float t)
{
	t = 0.0;
	float1 dist = 0.0;

	// squared length of line segment
	float1 lsq = (v.x - w.x)*(v.x - w.x) + (v.y - w.y)*(v.y - w.y);

	// v and w are the same point
	if (lsq == 0.0)
	{
		dist = length(p - v);
	}
	else
	{
		// project point onto line segment
		t = dot(p - v, w - v) / lsq;

		// point near v
		if (t < 0.0) dist = length(p - v);

		// point near w
		else if (t > 1.0) dist = length(p - w);

		// distance between p and projection point
		else dist = length(p - (v + t * (w - v)));
	}

	return dist;
}
