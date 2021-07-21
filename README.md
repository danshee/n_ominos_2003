# N-Omino Generator (2003)

This was a job interview screening problem I got in 2003 during a failed attempt to land a job at Research In Motion (now Blackberry).

**This is the problem as originally assigned to me:**

> You may or may not have heard of the pentomino puzzle.  The idea is typically to fit a bunch of pentominos (each made of 5 squares) into some shape, such as a 10x6 unit rectangle.
> 
> There are 12 unique pentoninos, if you allow for flips and rotates.
> 
> Writing a pentonino solver is an interesting problem, but solvers can be found at various spots on the web, so I can't ask you to do that.
> 
> http://math.hws.edu/xJava/PentominosSolver/
> 
> Instead, the challenge is to write a problem that generates the pentominos, using an argument, 'n', as the number of squares.  n=5 will generate pentominos, whereas n=6 generates hexominos, etc...  Obviously, the number of possible peces increases quite substantially with n.
> 
> You can assume that n shall only range from 1 to 7.  Write a program that generates the unique pentominos/hexominos/etc and prints them as ascii graphics on the screen.  Simple '#' vs spaces for occupied / unoccupied squares is sufficient.
> 
> Note that I have ideas as to how to solve this one, but haven't coded a solution, nor asked anybody to code a solution, so the problem isn't fully known to me in terms of difficulty.  I do think its a more interesting problem than the combos problem though.


**My response email contained the explanation:**

> Here is my solution.  It doesn't produce unique shapes, but it does uniquely produce all the orientations for each shape.
> 
> My solution essentially uses a brute-force recursion to generate all possible N-ominos and places each object into a linked list.  Each N-omino object stores the N-omino image in a 64-bit integer (i.e. 8-bits by 8-bits).  This 64-bit integer is used as a key to sort the objects.  The linked list is then uniqued using the 64-bit integer.  This cuts down on the duplicate shapes for a given orientation.
