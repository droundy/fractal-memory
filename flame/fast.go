package flame

import (
	"math"
	"fmt"
	"image"
	"image/color"
	"flag"
)

var Quality = flag.Float64("quality", 3, "quality of image")
var Brightness = flag.Float64("brightness", 4, "brightness of image")

type Pt struct {
	X, Y, R, G, B float64
}

type FastAffine struct {
	Mxx, Mxy, Myx, Myy, Ox, Oy float64
}

type TransformType uint8
const (
	AffineT TransformType = iota
  SwirlT
  HorseShoeT
	SinusoidalT
	PolarT
  CircleT
  SphericalT
)
func (t TransformType) String() string {
	switch t {
	case AffineT: return "Affine"
	case SwirlT: return "Swirl"
	case HorseShoeT: return "HorseShoe"
	case SinusoidalT: return "Sinusoidal"
	case PolarT: return "Polar"
	case SphericalT: return "Spherical"
	case CircleT: return "Circle"
	}
	return "unknown type"
}

type FastTransform struct {
	Type TransformType
	R, G, B float64
	Pre, Post FastAffine
}

func (i *FastAffine) Transform(p Pt) (o Pt) {
	o = p
	p.X += i.Ox
	p.Y += i.Oy
	o.X = p.X*i.Mxx + p.Y*i.Mxy
	o.Y = p.X*i.Myx + p.Y*i.Myy
	return o
}
func Distance(x, y float64) float64 {
	return math.Sqrt(x*x + y*y)
}

func (t *FastTransform) Transform22(p Pt) (o Pt) {
	o = t.Pre.Transform(p)
	o.R = 0.5*(p.R + t.R)
	o.G = 0.5*(p.G + t.G)
	o.B = 0.5*(p.B + t.B)
	switch t.Type {
	case AffineT:
		// Nothing to do
		return
	case HorseShoeT:
		r := Distance(o.X, o.Y)
		oX := (o.X - o.Y)*(o.X + o.Y)/r
		oY := 2*o.X*o.Y/(1+r)
		o.X = oX
		o.Y = oY
	case SwirlT:
		r2 := o.X*o.X + o.Y*o.Y
		sinr2 := math.Sin(r2)
		cosr2 := math.Cos(r2)
		oX := o.X*sinr2 - o.Y*cosr2
		oY := o.X*cosr2 + o.Y*sinr2
		o.X = oX
		o.Y = oY
	case SphericalT:
		r2 := o.X*o.X + o.Y*o.Y
		if r2 == 0 {
			return t.Post.Transform(o)
		}
		o.X /= 0.1 + r2
		o.Y /= 0.1 + r2
	case SinusoidalT:
		o.X = math.Sin(o.X)
		o.Y = math.Sin(o.Y)
	case PolarT:
		r := Distance(o.X, o.Y)
		theta := math.Atan2(o.Y, o.X)
		o.X = theta/math.Pi
		o.Y = r - 1
	case CircleT:
		r := Distance(o.X, o.Y)
		theta := math.Atan2(o.Y, o.X)
		o.X = r*math.Sin(r+theta)
		o.Y = r*math.Cos(r+theta)
	}
	o = t.Post.Transform(o)
	return
}

type FastSymmetry struct {
	A FastAffine
	N int
}

func (s *FastSymmetry) Transform(p Pt) Pt {
	o := s.A.Transform(p)
	o.X -= s.A.Ox
	o.Y -= s.A.Oy
	return o
}

type Flame struct {
	Transformations []FastTransform
	Symmetries []FastSymmetry
	R func() float64
	Next int
	TotSymmetries int
}

func (a *FastAffine) Init(r func() float64) {
	a.Mxx = r()
	a.Mxy = r()
	a.Myx = r()
	a.Myy = r()
	Mlen := math.Sqrt(math.Abs(a.Mxx*a.Myy - a.Mxy*a.Myx))
	if Mlen < 1.0 {
		a.Ox = r()*(1.0 - Mlen)
		a.Oy = r()*(1.0 - Mlen)
	} else {
		Mlen *= 1.1 // make it contractive
		a.Mxx /= Mlen
		a.Mxy /= Mlen
		a.Myx /= Mlen
		a.Myy /= Mlen
		a.Ox = 0
		a.Oy = 0
	}
}

func CreateFlame(ntrans, nsymm int, r func() float64) (f Flame) {
	f.R = r
	s := func() float64 { return (r() - 0.5)*2 }
	f.Transformations = make([]FastTransform, ntrans)
	f.Symmetries = make([]FastSymmetry, nsymm)
	for i := range(f.Transformations) {
		f.Transformations[i].Pre.Init(s)
		f.Transformations[i].Post.Init(s)
		myR := r()
		myG := r()
		myB := r()
		// The following bit should ensure full color saturation.
		maxC := myR
		if myG > maxC { maxC = myG }
		if myB > maxC { maxC = myB }
		minC := myR
		if myG < minC { minC = myG }
		if myB < minC { minC = myB }
		myR = (myR - minC)/(maxC - minC)
		myG = (myG - minC)/(maxC - minC)
		myB = (myB - minC)/(maxC - minC)
		f.Transformations[i].R = myR
		f.Transformations[i].G = myG
		f.Transformations[i].B = myB
		f.Transformations[i].Type = TransformType(float64(SphericalT+1)*r())
		fmt.Println(f.Transformations[i].Type)
	}
	f.TotSymmetries = 10
	for f.TotSymmetries > 5 {
		f.TotSymmetries = 1
		for i := range(f.Symmetries) {
			pIdentity := 2.0
			pInversion := pIdentity + 1.0
			pRotation := pInversion + 6
			pMirror := pRotation + 1
			switch p := r()*pMirror; {
			case p < pIdentity:
				fmt.Println("Identity")
				f.Symmetries[i].A.Mxx = 1
				f.Symmetries[i].A.Mxy = 0
				f.Symmetries[i].A.Myx = 0
				f.Symmetries[i].A.Myy = 1
				f.Symmetries[i].N = 1
			case p < pInversion:
				fmt.Println("Inversion")
				f.Symmetries[i].A.Mxx = -1
				f.Symmetries[i].A.Mxy = 0
				f.Symmetries[i].A.Myx = 0
				f.Symmetries[i].A.Myy = -1
				f.Symmetries[i].A.Ox = 0.8*s()
				f.Symmetries[i].A.Oy = 0.8*s()
				f.Symmetries[i].N = 2
			case p < pRotation:
				rootn := math.Sqrt(5)*0.5*(1+r())
				n := 1+int(rootn*rootn)
				if n < 2 {
					n = 2
				}
				fmt.Println("Rotation", n)
				theta := 2*math.Pi/float64(n)
				f.Symmetries[i].A.Mxx =  math.Cos(theta)
				f.Symmetries[i].A.Mxy =  math.Sin(theta)
				f.Symmetries[i].A.Myx = -math.Sin(theta)
				f.Symmetries[i].A.Myy =  math.Cos(theta)
				f.Symmetries[i].A.Ox = 0.5*s()
				f.Symmetries[i].A.Oy = 0.5*s()
				f.Symmetries[i].N = n
			case p < pMirror:
				fmt.Println("Mirror")
				x := s()
				y := s()
				dist := Distance(x,y)
				x /= dist
				y /= dist
				f.Symmetries[i].A.Mxx =  x
				f.Symmetries[i].A.Mxy =  y
				f.Symmetries[i].A.Myx =  y
				f.Symmetries[i].A.Myy = -x
				f.Symmetries[i].A.Ox = 0.8*s()
				f.Symmetries[i].A.Oy = 0.8*s()
				f.Symmetries[i].N = 2
			default:
				panic("This shouldn't happen")
			}
			f.TotSymmetries *= f.Symmetries[i].N
		}
	}
	return f
}

func (f *Flame) Transform(p Pt) (o Pt) {
	what_to_do := int(f.R()*float64(1+f.TotSymmetries))
	if what_to_do == 0 {
		n := int(f.R()*float64(len(f.Transformations)))
		return f.Transformations[n].Transform22(p)
	}
	what_to_do--
	for i := 0; i<len(f.Symmetries); i++ {
		if f.Symmetries[i].N > 0 {
			if f.R() > 1.0/float64(f.Symmetries[i].N) {
				p = f.Symmetries[i].Transform(p)
			}
		}
	}
	return p
}

func (f *Flame) Run(size int) image.Image {
	histR := make([]float64, size*size)
	histG := make([]float64, size*size)
	histB := make([]float64, size*size)
	histA := make([]float64, size*size)

	hits := 0.0
	misses := 0.0
	wanthits := math.Pow(10, *Quality)*float64(size)*float64(size)
	notifyme := wanthits/100

	p := Pt{ f.R(), f.R(), 0, 0, 0 }
	for i:=0;i<10000;i++ {
		p = f.Transform(p)
	}

	for hits < wanthits {
		if hits > notifyme {
			fmt.Printf("%2.0f%% done!\r", 100*notifyme/wanthits)
			notifyme += wanthits/100
		}
		p = f.Transform(p)
		if p.X != p.X || p.Y != p.Y || p.R != p.R {
			fmt.Println("NaN:", p.X, p.Y, p.R)
			panic("nan")
		}
		xint := int((p.X + 1.0)*0.5*float64(size))
		yint := int((p.Y + 1.0)*0.5*float64(size))
		n := xint + size*yint
		if n >= 0 && n < size*size {
			histR[n] += p.R
			histG[n] += p.G
			histB[n] += p.B
			histA[n] += 1
			hits++
		} else {
			misses++
		}
		if misses > 1000 && misses < 2000 && hits/misses < 0.5 {
			fmt.Printf("\nGiving up with ratio %g!\n\n", hits/misses)
			return nil
		}
	}
	im := image.NewNRGBA(image.Rect(0,0,size,size))
	maxA := 0.0
	minA := 1.0
	for i := 0; i < size*size; i++ {
		if histA[i] > maxA {
			maxA = histA[i]
		}
		if histA[i] > 0 && histA[i] < minA {
			minA = histA[i]
		}
	}
	for ix := 0; ix < size; ix++ {
		for iy := 0; iy < size; iy++ {
			n := ix + size*iy
			if histA[n] > 0 {
				a := math.Log(histA[n]/minA* *Brightness)/math.Log(maxA/minA* *Brightness)/histA[n]
				//fmt.Println(ix, iy, a, histA[n])
				im.Set(ix, iy, RGB(histR[n]*a, histG[n]*a, histB[n]*a))
			} else {
				im.Set(ix, iy, RGB(0, 0, 0))
			}
		}
	}
	fmt.Println("hit/miss ratio:", hits/misses)
	return im
}

func RGB(r, g, b float64) color.Color {
	if r >= 1 {
		r = 0.999999
	}
	if g >= 1 {
		g = 0.999999
	}
	if b >= 1 {
		b = 0.999999
	}
	if r < 0 {
		r = 0
	}
	if g < 0 {
		g = 0
	}
	if b < 0 {
		b = 0
	}
	return color.NRGBA{ uint8(r*256), uint8(g*256), uint8(b*256), 255 }
}
