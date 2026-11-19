/ HERMETIC-Ω pure Q/K vector geometric engine
/ Compact array evaluation

norm:{sqrt sum x*x}
dot:{sum x*y}
normalize:{x%norm x}

sphere_exp:{
  x:x; v:y;
  nv:sqrt sum v*v;
  c:cos nv;
  s:sin[nv]%nv;
  (c*x)+s*v
}

evolve:{
  x:x; v:y; eta:z; fuel:w;
  $[fuel=0;x;evolve[sphere_exp[x;eta*v];v;eta;fuel-1]]
}
