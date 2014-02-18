Code choices:
============

* No conditional that affects the histograms should be affected by a
  floating-point computation.  This means that our code should be
  unaffected by floating point roundoff error.

* [TODO] No use of rand().  We want our code to give identical output
  on any platform.  Replace with (a) sequential SHA-512 in creating
  Flame structure, followed by simple and fast PRNG for simulation.


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

- How does *shape* distinguishability depend on colors themselves? Is
  CIELUV or CIELAB distance relevant here?

- How many distinguishable shapes can we generate by changing just one
  parameter, for each possible parameter? How does this entropy depend
  on parameter details, i.e. are there combinations we should avoid,
  because they make the image redundant?

- How much worse is a gray-scale flame?

Larger questions:
================

* Compare fractal flames with other hash-like approaches:

  - Random art (see paper)
  - ssh random art (different, see openssh)
  - Flag
  - other flags
  - hexadecimal
  - formatted hexadecimal (e.g. with spaces, see ssh or openpgp
    fingerprint)
  - sequence of words (see other paper)

* Measure retention as a function of human-affecting
  information-unaffecting tweaks, e.g. the effect of

  - image size (particularly relevant for applications)
  - image color scheme (e.g. black background vs white background)
  - screen background apart from image
  - font size
  - font choice or formatting
  - font color (black on white vs. white on black)

* Measure retention as a function of confounding factors

  - time delay
  - distracting displays (e.g. show cute kittens after initial object,
    before comparisons)
  - distracting sounds
  - short time viewing original
  - time limit to identify test images

"Game" types:
============

*** Simple delay and match: ***
- Show original
- Show sequence of test images
- User identifies which images match the original as they go

Multiple delay and match:
- Show several originals
- Show sequence of test images (we need good terminology for these)
- User identifies which images match at least one original

Multiple delay and identify:
- Show several originals
- Show sequence of test images (we need good terminology for these)
- User identifies which images match which original (direction? number?)

Simple compare:
- Show user two images
- User identifies if they match each other

*** Pair identification: ***
- Show user a grid of many images
- User picks out and identifies matching pairs
- [Mostly useful for small image sizes, benefits from birthday paradox]
