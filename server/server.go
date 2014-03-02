package main

import (
	"os"
	"io"
	"net"
	"fmt"
	"strings"
	"time"
)

func handleConnection(c net.Conn) {
	defer c.Close()
	var name string
	n, err := fmt.Fscanln(c, &name)
	if n != 1 || err != nil {
		fmt.Println("unable to read from conn: ", c.RemoteAddr(), n, err)
		return
	}
	if strings.ContainsAny(name, "/ \t\n") || strings.Count(name, "..") > 0 {
		fmt.Println("bad name given: ", name)
		return
	}
	os.MkdirAll("data/" + name, 0777)
	f, err := os.Create("data/" + name + "/" + time.Now().Format("2006-01-02-15-04-05"))
	if err != nil {
		fmt.Println("unable to create file! ", err)
		return
	}
	defer f.Close()
	fmt.Println("Connection from ", name, " on IP address ", c.RemoteAddr())
	io.Copy(f, c)
}

func main() {
	ln, err := net.Listen("tcp", ":5432")
	if err != nil {
		// handle error
		fmt.Println("Unable to create socket!\n");
		os.Exit(1)
	}
	for {
		conn, err := ln.Accept()
		if err != nil {
			// handle error
			continue
		}
		fmt.Println("reading from conn!\n", conn.RemoteAddr())
		go handleConnection(conn)
	}
}
