struct vector2
{
	real32 X;
	real32 Y;
};

inline vector2 
operator + (vector2 A, vector2 B)
{
	vector2 Result;

	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;

	return (Result);
}

inline vector2
operator += (vector2 &A, vector2 B)
{
	A = A + B;

	return (A);
}

inline vector2
operator * (vector2 A, real32 B)
{
	vector2 Result;

	Result.X = A.X * B;
	Result.Y = A.Y * B;

	return (Result);
}

inline vector2
operator * (real32 B, vector2 A)
{
	vector2 Result;

	Result.X = A.X * B;
	Result.Y = A.Y * B;

	return (Result);
}

inline vector2
operator - (vector2 A, vector2 B)
{
	vector2 Result;

	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;

	return (Result);
}