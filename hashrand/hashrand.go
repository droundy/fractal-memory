package hashrand

import (
	"crypto/sha512"
	"crypto/aes"
	"crypto/cipher"
	"fmt"
)

// The next 8 lines are translated from the Wikipedia page on random
// number generation.
var m_w uint32 = 1;    // must not be zero, nor 0x464fffff
var m_z uint32= 2;    // must not be zero, nor 0x9068ffff

func MultiplyWithCarryRand() int {
  m_z = 36969 * (m_z & 65535) + (m_z >> 16);
  m_w = 18000 * (m_w & 65535) + (m_w >> 16);
  v := int((m_z << 16) + m_w);  // 32-bit result
	if v < 0 {
		v = -v
	}
	return v
}

func Rand(s string) (func() float64) {
	hh := sha512.New()
	hh.Write([]byte(s))
	h := hh.Sum(nil)
	fmt.Printf("Hash %s: %x\n", s, h)
	key := h[0:32]
	iv := h[32:32+16]
	block, err := aes.NewCipher(key)
	if err != nil {
		panic(err)
	}
	stream := cipher.NewCFBEncrypter(block, iv)
	return func() float64 {
		x := make([]byte, 4)
		stream.XORKeyStream(x,x)
		return (float64(x[0]) + 256*float64(x[1]) + 256*256*float64(x[2]) +
			256*256*256*float64(x[3]))/(256*256*256*256)
	}
	return func() float64 {
		x := make([]byte, 2)
		stream.XORKeyStream(x,x)
		return (float64(x[0]) + 256*float64(x[1]))/(256*256)
	}
}
