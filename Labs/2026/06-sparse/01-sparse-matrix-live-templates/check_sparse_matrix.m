% Reconstruct the sparse matrix from triplets and verify A * x.

A = spconvert(
[ 1 , 1, 4;
 1 , 2, -1;
 2 , 1, -1;
 2 , 2, 4;
 2 , 3, -1;
 3 , 2, -1;
 3 , 3, 4;
 3 , 4, -1;
 4 , 3, -1;
 4 , 4, 4;
 4 , 5, -1;
 5 , 4, -1;
 5 , 5, 4;
 5 , 6, -1;
 6 , 5, -1;
 6 , 6, 4;
 6 , 7, -1;
 7 , 6, -1;
 7 , 7, 4;
 7 , 8, -1;
 8 , 7, -1;
 8 , 8, 4;
 8 , 9, -1;
 9 , 8, -1;
 9 , 9, 4;
 9 , 10, -1;
 10 , 9, -1;
 10 , 10, 4;
 ]
);

x = ones(10, 1);
b = A * x;

disp("A (sparse):");
disp(A);

disp("A (full):");
disp(full(A));

disp("x:");
disp(x);

disp("A * x:");
disp(b);
