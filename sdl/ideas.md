
* No conditional that affects the histograms should be affected by a
  floating-point computation.  This means that our code should be
  unaffected by floating point roundoff error.

* [TODO] No use of rand().  We want our code to give identical output
  on any platform.


Questions:
=========

- What is the optimal meanr scaling factor?

- If we fix the colors, how hard is it to distinguish different shapes?

- If the picture is monochrome, how hard is it to distinguish different shapes?

- If we fix the shape, how hard is it to distinguish different colors?
  How does this difficulty depend on details of the fractal? i.e. can
  we find a pattern of hard-to distinguish shapes?

- How does color distinguishability depend on colors themselves? Is
  CIELUV or CIELAB distance relevant here?

- How many distinguishable shapes can we generate by changing just one
  parameter, for each possible parameter? How does this entropy depend
  on parameter details, i.e. are there combinations we should avoid,
  because they make the image redundant?
