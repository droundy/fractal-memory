package main

import (
	"./flame"
	"os"
	"fmt"
	"time"
	"flag"
	"image"
	"image/jpeg"
	"image/png"
	"math/rand"
)

var seed = flag.Int64("seed", 0, "random number seed")
var size = flag.Int("size", 100, "size of graphic")

func main() {
	flag.Parse()
	if *seed == 0 {
		t := time.Now().UTC().UnixNano()/1000 % 10000
		fmt.Println("Seeding with time...", t)
		rand.Seed(t)
	} else {
		rand.Seed( *seed )
	}
	var i image.Image
	for i == nil {
		f := flame.CreateFlame(4, 2, rand.Float64)
		i = f.Run(*size)
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

	p2, err := os.Create(fmt.Sprintf("avatar-seed-%d-size-%d-bright-%f-quality-%f.png",
		*seed, *size, *flame.Brightness, *flame.Quality))
	if err != nil {
		fmt.Println("Error creating file: ", err)
		os.Exit(1)
	}
	defer p2.Close()
	png.Encode(p, i)
}
