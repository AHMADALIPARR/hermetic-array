NB. HERMETIC-Ω pure J tacit geometric engine
NB. Rank conjunctions, forks, hooks

norm =: +/ . *
dot =: +/ . *
normalize =: ] % %: norm

sphere_exp =: 3 : 0
  'x v' =. y
  nv =. %: norm v
  c =. 2 o. nv
  s =. (1 o. nv) % nv
  (c * x) + (s * v)
)

geodesic =: 3 : 0
  'x y t' =. y
  theta =. _2 o. dot x,y
  s0 =. 1 o. (1-t)*theta
  s1 =. 1 o. t*theta
  ((s0*x) + (s1*y)) % 1 o. theta
)

NB. Recursive evolution with power
evolve =: 4 : 0
  'x v eta n' =. y
  n = 0 do. x return. end.
  sphere_exp (x ; (eta*v))
)
