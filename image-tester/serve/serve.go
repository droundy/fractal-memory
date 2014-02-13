package serve

import (
	"net/http"
	"log"
	"image/png"
	"strconv"
	"../../flame"
	"../../hashrand"
)

func withInt(k string, values map[string][]string, f func(int)) {
	if strs, ok := values[k]; ok {
		if len(strs) == 1 {
			v, err := strconv.Atoi(strs[0])
			if err == nil {
				log.Println("Got", k, v)
				f(v)
			}
		}
	}
}

func withFloat64(k string, values map[string][]string, f func(float64)) {
	if strs, ok := values[k]; ok {
		if len(strs) == 1 {
			v, err := strconv.ParseFloat(strs[0], 64)
			if err == nil {
				log.Println("Got", k, v)
				f(v)
			}
		}
	}
}

func handleFlame(w http.ResponseWriter, r *http.Request) {
	f := flame.CreateFlame(4, 2, hashrand.Rand(r.URL.Path))
	values := r.URL.Query()
	log.Println(r.URL.Path)
	log.Println("Query: ", values)
	size := 50
	withInt("size", values, func(v int) {
		size = v
	})
	quality := 2.0
	withFloat64("quality", values, func(v float64) {
		quality = v
	})
	i := f.Run(size, 2)
	png.Encode(w, i)
}

func HandleFlames() {
	http.HandleFunc("/flames/", handleFlame)
}
