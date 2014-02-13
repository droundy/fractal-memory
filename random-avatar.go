package main

import (
	"./flame"
	"./hashrand"
	"os"
	"fmt"
	"time"
	"flag"
	"image"
	"image/jpeg"
	"image/png"
	"math/rand"
)

var seed = flag.String("seed", "", "random number seed")
var size = flag.Int("size", 100, "size of graphic")
var quality = flag.Float64("quality", 3, "quality of image")

func main() {
	flag.Parse()
	if *seed == "" {
		t := ":" + Things[(time.Now().UTC().UnixNano()/1000) % int64(len(Things))]
		fmt.Println("Seeding with time-based... ", t)
		*seed = t
	}
	//rand.Seed( *seed )
	var i image.Image
	for i == nil {
		var f flame.Flame
		if true {
			f = flame.CreateFlame(4, 2, hashrand.Rand(fmt.Sprint( *seed )))
		} else {
			f = flame.CreateFlame(4, 2, rand.Float64)
		}
		i = f.Run(*size, *quality)
	}

	f, err := os.Create("avatar.jpg")
	if err != nil {
		fmt.Println("Error creating file: ", err)
		os.Exit(1)
	}
	defer f.Close()
	p, err := os.Create("avatar.png")
	if err != nil {
		fmt.Println("Error creating file: ", err)
		os.Exit(1)
	}
	defer p.Close()
	//i := avatar.FractalImage(400, 300)
	//i = avatar.Avatar(400, 300)

	jpeg.Encode(f, i, &jpeg.Options{100})
	png.Encode(p, i)

	p2, err := os.Create(fmt.Sprintf("avatar-seed-%d-size-%d-quality-%f.png",
		*seed, *size, *quality))
	if err != nil {
		fmt.Println("Error creating file: ", err)
		os.Exit(1)
	}
	defer p2.Close()
	png.Encode(p, i)
}
