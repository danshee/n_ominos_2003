//***************************************************************************
//
//  Filename: n_ominos.cpp
//
//  Description:
//    This program generates n-ominoes, for n = [1 .. 7].
//
//***************************************************************************


/*................................. INCLUDES ..............................*/

#include <stdio.h>
#include <list>

/*.................................. MACROS ...............................*/

#define ARRAY_LENGTH(A)          (sizeof(A)/sizeof(A[0]))

#ifndef max
#define max(a,b)                  (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)                  (((a) < (b)) ? (a) : (b))
#endif

/*.................................. GLOBALS ..............................*/

/*................................... TYPES ...............................*/

typedef unsigned char            uint8;
typedef unsigned __int64          uint64;


// This type represents one possible combination of each direction (North,
// East, South, West).
struct DIR_COMB
{
  int combo;    // The combination.
  int squares;  // The number of square used by this combination.
};


// Directions are North, East, South and West (used in drawing).
enum DIR
{
  DIR_N  =  1 << 0,  // North
  DIR_E  =  1 << 1,  // East
  DIR_S  =  1 << 2,  // South
  DIR_W  =  1 << 3,  // West

  DIR_NUM,
};


// Vector type used for drawing.
struct VECT
{
  int dx;
  int dy;
};


// Point type used for drawing.
struct POINT
{
  int x;
  int y;
};


/*................................. CONSTANTS .............................*/

// Limit on number of squares.
const kLimit = 7;


// These are all the possible combinations of the directions, as well as
// how many squares are involved in each combination.
const DIR_COMB kDirComb[] =
{
  {                          DIR_N,    1 },
  {                  DIR_E        ,    1 },
  {                  DIR_E | DIR_N,    2 },
  {          DIR_S                ,    1 },
  {          DIR_S        | DIR_N,    2 },
  {          DIR_S | DIR_E        ,    2 },
  {          DIR_S | DIR_E | DIR_N,    3 },
  {  DIR_W |                DIR_N,    2 },
  {  DIR_W |        DIR_E        ,    2 },
  {  DIR_W |        DIR_E | DIR_N,    3 },
  {  DIR_W | DIR_S                ,    2 },
  {  DIR_W | DIR_S        | DIR_N,    3 },
  {  DIR_W | DIR_S | DIR_E        ,    3 },
  {  DIR_W | DIR_S | DIR_E | DIR_N,    4 },
};


// The total number of possible direction combinations.
const kNumDirComb = ARRAY_LENGTH(kDirComb);


// Vectors for translating North, East, South, West into grid vectors.
const VECT kVecNorth = {  0, -1  };
const VECT kVecEast  = {  1,  0  };
const VECT kVecSouth = {  0,  1  };
const VECT kVecWest  = { -1,  0  };


/*............................ FUNCTION PROTOTYPES ........................*/

/*............................. CLASS DEFINITIONS .........................*/


// This class implements an 8=bit by 8-bit grid in which individual bits
// can be set to 1 or their value retrieved.
class Grid8x8
{
  union BITS
  {
    uint64  ui64;
    uint8  ui8[8];
  };

  BITS _bits;

  int Normalize(int i) const;
  void Normalize(POINT& pt) const;

public:
  Grid8x8();

  void Set(POINT pt);
  bool Get(POINT pt) const;

  int Comp(const Grid8x8& a) const;

  void Translate(VECT vec);

}; // class Grid8x8


class N_Omino
{
  // Grid of squares.
  Grid8x8 _grid;

  // (x,y) position of current square.
  POINT _pos;

  // The extents define a rectangle within which all the squares of
  // the n-omino reside.
  POINT _extent1, _extent2;

  // This is the number of free squares left.
  int _squaresLeft;

  bool Add(const VECT& kVec);
  Follow(const VECT& kVec);

public:
  N_Omino(int N);

  bool AddNorth();
  bool AddEast();
  bool AddSouth();
  bool AddWest();

  FollowNorth();
  FollowEast();
  FollowSouth();
  FollowWest();

  Draw(int row) const;
  Draw() const;

  Normalize();

  int Comp(const N_Omino& a) const;

  bool operator<(const N_Omino& a) const;
  bool operator==(const N_Omino& a) const;

  int SquaresLeft() const;

}; // class N_Omino


/*.............................. IMPLEMENTATION ...........................*/


//***********************************************************************
//
// Function:    operator+()
// Description:  Added a vector to a point, resulting in a new point.
//
//***********************************************************************

POINT operator+(const POINT& p, const VECT& v)
{
  POINT tmp = { p.x + v.dx, p.y + v.dy };
  return tmp;
}


//***********************************************************************
//
// Function:    RightRotate8()
// Description:  Rotate an 8-bit integer to the right.  Rotation means
//              that bits are shifted, and  those bits that fall off
//              one end are pushed back onto the other end.
// Parameters:  val : the 8-bit value to be rotated.
//              rot : the number of bits by which to rotate the bits
//                    of val (0 - 7).
// Returns:      The rotated value.
//
//***********************************************************************
uint8 RightRotate8(uint8 val, int rot)
{
  uint8 tmp1 = val;
  uint8 tmp2 = val;

  tmp1 = val >> rot;
  tmp2 = val << (8 - rot);

  return tmp1 | tmp2;

} // RightRotate8


//***********************************************************************
//
// Function:    LeftRotate64()
// Description:  Rotate a 64-bit integer to the left.  Rotation means
//              that bits are shifted, and  those bits that fall off
//              one end are pushed back onto the other end.
// Parameters:  val : the 64-bit value to be rotated.
//              rot : the number of bits by which to rotate the bits
//                    of val (0 - 63).
// Returns:      The rotated value.
//
//***********************************************************************
uint64 LeftRotate64(uint64 val, int rot)
{
  uint64 tmp1 = val;
  uint64 tmp2 = val;

  tmp1 = val << rot;
  tmp2 = val >> (64 - rot);

  return tmp1 | tmp2;

} // LeftRotate64


//***********************************************************************
//
// Function:    Grid8x8::Grid8x8()
// Description:  Sets all points in the grid to 0.
//
//***********************************************************************
Grid8x8::Grid8x8()
{
  _bits.ui64 = 0;
} // Grid8x8::Grid8x8


//***********************************************************************
//
// Function:    Grid8x8::Normalize()
// Description:  Grid8x8 uses this method to wrap around negative indices.
//
//***********************************************************************
int Grid8x8::Normalize(int i) const
{
  return (8 + i) % 8;
} // Normalize


//***********************************************************************
//
// Function:    Grid8x8::Normalize()
// Description:  Normalize a POINT by applying Normalize() to x and y.
//
//***********************************************************************
void Grid8x8::Normalize(POINT& pt) const
{
  pt.x = Normalize(pt.x);
  pt.y = Normalize(pt.y);
} // Normalize


//***********************************************************************
//
// Function:    Grid8x8::Set()
// Description:  Sets the (x,y) coordinate in the grid to 1.
//
//***********************************************************************
void Grid8x8::Set(POINT pt)
{
  Normalize(pt);
  _bits.ui8[pt.y] |= (0x80 >> pt.x);

} // Grid8x8::Set


//***********************************************************************
//
// Function:    Grid8x8::Get()
// Description:  Retrieves the state of the (x,y) point in the grid.
//
//***********************************************************************
bool Grid8x8::Get(POINT pt) const
{
  Normalize(pt);
  return (_bits.ui8[pt.y] & (0x80 >> pt.x)) ? true : false;

} // Grid8x8::Get


//***********************************************************************
//
// Function:    Grid8x8::Translate()
// Description:  For each point P in the grid, the point value is
//              moved to P + V, where V is 'vec'.
// Parameters:  vec : the vector V.
//
//***********************************************************************
void Grid8x8::Translate(VECT vec)
{
  // Translate along x-axis.
  for (int i = 0; i < 8; ++i)
  {
    _bits.ui8[i] = RightRotate8(_bits.ui8[i], vec.dx);
  } // for i

  // Translate along y-axis.
  _bits.ui64 = LeftRotate64(_bits.ui64, 8*vec.dy);

} // Grid8x8::Translate


//***********************************************************************
//
// Function:    Grid8x8::Comp()
// Description:  Compares a Grid8x8 object with *this.
// Parameters:  a : another Grid8x8.
// Returns:      0 if *this is lexicographically equal, -1 or +1
//              otherwise (_bits.ui64 is used to compare).
//
//***********************************************************************
int Grid8x8::Comp(const Grid8x8& a) const
{
  if (_bits.ui64 < a._bits.ui64)
  {
    return -1;
  }
  else if (_bits.ui64 > a._bits.ui64)
  {
    return +1;
  }
  else
  {
    return 0;
  }
} // Grid8x8::Comp


//***********************************************************************
//
// Function:    N_Omino::N_Omino()
// Description:  Construct an N_Omino object.
//
//***********************************************************************
N_Omino::N_Omino(int N)
{
  _pos.x = _pos.y = 0;
  _extent1 = _extent2 = _pos;

  _grid.Set(_pos);

  _squaresLeft = N - 1;

} // N_Omino::N_Omino


//***********************************************************************
//
// Function:    N_Omino::Add()
// Description:  Implements the N_Omino::Add<direction> methods by
//              adding a new square at the current position plus the
//              supplied vector.
// Parameters:  kVec : the direction vector.
// Returns:      false : a collision occurred (i.e. a square already
//                      occupies the new position) or the current
//                      object has N squares.
//              true : the square was successfully added.
//
//***********************************************************************
bool N_Omino::Add(const VECT& kVec)
{
  POINT pos = _pos + kVec;

  if (_grid.Get(pos) || (_squaresLeft < 1))
  {
    // Collision!
    return false;
  }
  else
  {
    _grid.Set(pos);

    _extent1.x = min(pos.x, _extent1.x);
    _extent1.y = min(pos.y, _extent1.y);

    _extent2.x = max(pos.x, _extent2.x);
    _extent2.y = max(pos.y, _extent2.y);

    --_squaresLeft;

    return true;
  }
} // N_Omino::Add


//***********************************************************************
//
// Function:    N_Omino::AddNorth()
// Description:  See N_Omino::Add().
//
//***********************************************************************
bool N_Omino::AddNorth()
{
  return Add(kVecNorth);
} // N_Omino::AddNorth


//***********************************************************************
//
// Function:    N_Omino::AddEast()
// Description:  See N_Omino::Add().
//
//***********************************************************************
bool N_Omino::AddEast()
{
  return Add(kVecEast);
} // N_Omino::AddEast


//***********************************************************************
//
// Function:    N_Omino::AddSouth()
// Description:  See N_Omino::Add().
//
//***********************************************************************
bool N_Omino::AddSouth()
{
  return Add(kVecSouth);
} // N_Omino::AddSouth


//***********************************************************************
//
// Function:    N_Omino::AddWest()
// Description:  See N_Omino::Add().
//
//***********************************************************************
bool N_Omino::AddWest()
{
  return Add(kVecWest);
} // N_Omino::AddWest


//***********************************************************************
//
// Function:    N_Omino::Follow()
// Description:  Implements the N_Omino::Follow<direction> methods by
//              adding the supplied vector to current position.  This
//              method adds a new square to this object.
// Parameters:  kVec : the direction vector.
//***********************************************************************
N_Omino::Follow(const VECT& kVec)
{
  _pos = _pos + kVec;
} // N_Omino::Follow


//***********************************************************************
//
// Function:    N_Omino::FollowNorth()
// Description:  See N_Omino::Follow();
//
//***********************************************************************
N_Omino::FollowNorth()
{
  Follow(kVecNorth);
} // N_Omino::FollowNorth


//***********************************************************************
//
// Function:    N_Omino::FollowEast()
// Description:  See N_Omino::Follow();
//
//***********************************************************************
N_Omino::FollowEast()
{
  Follow(kVecEast);
} // N_Omino::FollowEast


//***********************************************************************
//
// Function:    N_Omino::FollowSouth()
// Description:  See N_Omino::Follow();
//
//***********************************************************************
N_Omino::FollowSouth()
{
  Follow(kVecSouth);
} // N_Omino::FollowSouth


//***********************************************************************
//
// Function:    N_Omino::FollowWest()
// Description:  See N_Omino::Follow();
//
//***********************************************************************
N_Omino::FollowWest()
{
  Follow(kVecWest);
} // N_Omino::FollowWest


//***********************************************************************
//
// Function:    N_Omino::Draw()
// Description:  Draw one row of the n-omino encapsulated by this object.
//
//***********************************************************************
N_Omino::Draw(int row) const
{
  POINT pt = { 0, row };

  for (int i = _extent1.x; i <= _extent2.x; ++i)
  {
    pt.x = i;
    printf("%s", _grid.Get(pt) ? "[]" : "  ");
  } // for

} // N_Omino::Draw


//***********************************************************************
//
// Function:    N_Omino::Draw()
// Description:  Draw the n-omino encapsulated by this object.
//
//***********************************************************************
N_Omino::Draw() const
{
  printf("\n\n");

  for (int i = _extent1.y; i <= _extent2.y; ++i)
  {
    Draw(i);
    printf("\n");
  } // for i

} // N_Omino::Draw()


//***********************************************************************
//
// Function:    N_Omino::Normalize()
// Description:  The n-omino is essentially a set of five (x,y)
//              coordinates, some of which may be negative.  This
//              method "normalizes" the coordinates of the n-omino
//              encapsulated by this object by translating each
//              point such that the minimal x and y value in all
//              points is 0.
//
//***********************************************************************
N_Omino::Normalize()
{
  VECT trans = { -_extent1.x, -_extent1.y };
  _grid.Translate(trans);

  _extent2.x -= _extent1.x;
  _extent1.x = 0;

  _extent2.y -= _extent1.y;
  _extent1.y = 0;

} // N_Omino::Normalize


//***********************************************************************
//
// Function:    N_Omino::Comp()
// Description:  Compares a N_Omino object with *this.
// Parameters:  a : another N_Omino.
// Returns:      0 if *this is lexicographically equal, -1 or +1
//              otherwise (Grid8x8::Comp() is used to compare).
//
//***********************************************************************
int N_Omino::Comp(const N_Omino& a) const
{
  return _grid.Comp(a._grid);
} // N_Omino:: Comp


//***********************************************************************
//
// Function:    N_Omino::operator<()
// Description:  Returns true of *this is lexicographically less than a.
// Parameters:  a : another N_Omino.
//
//***********************************************************************
bool N_Omino::operator<(const N_Omino& a) const
{
  return Comp(a) < 0;
} // N_Omino::operator<


//***********************************************************************
//
// Function:    N_Omino::operator<()
// Description:  Returns true of *this is lexicographically equal to a.
// Parameters:  a : another N_Omino.
//
//***********************************************************************
bool N_Omino::operator==(const N_Omino& a) const
{
  return Comp(a) == 0;
} // N_Omino::operator==


//***********************************************************************
//
// Function:    N_Omino::SquaresLeft()
// Description:  Returns the number of squares that still need to be
//              added to the current state of this object to make it
//              a complete N-omino..
//
//***********************************************************************
int N_Omino::SquaresLeft() const
{
  return _squaresLeft;
} // N_Omino::SquaresLeft


//***********************************************************************
//
// Function:    N_OminosDraw()
// Description:  Draw each N_Omino object in the collection.
//
//***********************************************************************
template<class T>
void N_OminosDraw(const T& n_ominos)
{
  int n = 0;

  T::const_iterator i;
  for (i = n_ominos.begin(); i != n_ominos.end(); ++i)
  {
    i->Draw();
  } // for i
} // N_OminosDraw


//***********************************************************************
//
// Function:    N_OminosGenerateImp()
// Description:  Recursively generate all n-ominos in their orientations.
// Parameters:  n_ominos : a collection to which new N_Omino objects are
//                          added.
//              n_omino :  a potentially incomplete N_Omino generated
//                          by the previous iteration.  This iteration
//                          either decide that this N_Omino is complete
//                          and add it to n_ominos, or it will attempt
//                          to add one or more squares to continue the
//                          iteration.
//
//***********************************************************************
template<class T>
void N_OminosGenerateImp(T& n_ominos, N_Omino n_omino)
{
  if (n_omino.SquaresLeft() == 0)
  {
    // N squares have been added -- n_omino is complete.
    n_omino.Normalize();
    n_ominos.push_back(n_omino);
    return;
  }

  // For each combination of DIR.
  for (int i = 0; i < kNumDirComb; ++i)
  {
    N_Omino newNOmino = n_omino;

    // Add new squares according to kDirComb[i] to the current iteration
    // of n_omino, creating a new N_Omino object.

    if (kDirComb[i].combo & DIR_N)
    {
      if (!newNOmino.AddNorth())
      {
        // Collision.
        continue;
      }
    }

    if (kDirComb[i].combo & DIR_E)
    {
      if (!newNOmino.AddEast())
      {
        // Collision.
        continue;
      }
    }

    if (kDirComb[i].combo & DIR_S)
    {
      if (!newNOmino.AddSouth())
      {
        // Collision.
        continue;
      }
    }

    if (kDirComb[i].combo & DIR_W)
    {
      if (!newNOmino.AddWest())
      {
        // Collision.
        continue;
      }
    }

    // Each new square added represents a potential for expanding
    // the current (possibly incomplete) N_Omino object, follow each
    // one recursively.

    if (kDirComb[i].combo & DIR_N)
    {
      N_Omino anotherNOmino = newNOmino;
      anotherNOmino.FollowNorth();
      N_OminosGenerateImp(n_ominos, anotherNOmino);
    }

    if (kDirComb[i].combo & DIR_E)
    {
      N_Omino anotherNOmino = newNOmino;
      anotherNOmino.FollowEast();
      N_OminosGenerateImp(n_ominos, anotherNOmino);
    }

    if (kDirComb[i].combo & DIR_S)
    {
      N_Omino anotherNOmino = newNOmino;
      anotherNOmino.FollowSouth();
      N_OminosGenerateImp(n_ominos, anotherNOmino);
    }

    if (kDirComb[i].combo & DIR_W)
    {
      N_Omino anotherNOmino = newNOmino;
      anotherNOmino.FollowWest();
      N_OminosGenerateImp(n_ominos, anotherNOmino);
    }
  } // for i

} // N_OminosGenerateImp


//***********************************************************************
//
// Function:    N_OminosGenerate()
// Description:  Generate all n-ominos consisting of N squares.
// Parameters:  N : the number of squares per n-omino.
// Returns:      0 if N is outside the range [1..7].
//
//***********************************************************************
int N_OminosGenerate(const N)
{
  if ((1 <= N) && (N <= kLimit))
  {
    std::list<N_Omino> n_ominos;

    // Add the first one.
    N_Omino n_omino(N);

    N_OminosGenerateImp(n_ominos, n_omino);

    n_ominos.sort();
    n_ominos.unique();

    printf("n_ominoes = %u\n", n_ominos.size());

    N_OminosDraw(n_ominos);

    return 1;
  }

  return 0;

} // N_OminosGenerate



//***********************************************************************
//
// Function:    main()
// Parameters:  argv[1] : the number of squares per n-omino.
//
//***********************************************************************
int main(int argc, char* argv[])
{
  if (argc == 2)
  {
    const N = atoi(argv[1]);

    if (N_OminosGenerate(N))
    {
      return 1;
    }
  }

  printf("Usage: n_ominoes <1-7>\n");

    return 0;

} // main

