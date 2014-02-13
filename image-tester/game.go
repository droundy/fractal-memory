package main

import (
	"code.google.com/p/gowut/gwu"
	"fmt"
)

func buildGameWin(s gwu.Session) {
	win := gwu.NewWindow("game", "Game Window")
	win.Style().SetFullSize()
	win.SetAlign(gwu.HA_CENTER, gwu.VA_MIDDLE)

	p := gwu.NewPanel()
	p.SetHAlign(gwu.HA_CENTER)
	p.SetCellPadding(2)

	l := gwu.NewLabel("Image Tester")
	l.Style().SetFontWeight(gwu.FONT_WEIGHT_BOLD).SetFontSize("150%")
	p.Add(l)
	l = gwu.NewLabel("Game")
	l.Style().SetFontWeight(gwu.FONT_WEIGHT_BOLD).SetFontSize("120%")
	p.Add(l)
	//p.CellFmt(l).Style().SetBorder2(1, gwu.BRD_STYLE_DASHED, gwu.CLR_NAVY)
	// l = gwu.NewLabel("user/pass: admin/a")
	// l.Style().SetFontSize("80%").SetFontStyle(gwu.FONT_STYLE_ITALIC)
	// p.Add(l)

	errL := gwu.NewLabel("")
	errL.Style().SetColor(gwu.CLR_RED)
	p.Add(errL)

	table := gwu.NewTable()
	table.SetCellPadding(2)
	table.EnsureSize(3, 3)
	table.Add(gwu.NewLabel("User name:"), 0, 0)
	tb := gwu.NewTextBox("foobar")
	tb.Style().SetWidthPx(160)
	table.Add(tb, 0, 1)

	avatar := gwu.NewImage("/flames/", "/flames/")
	tb.AddEHandlerFunc(func(e gwu.Event) {
		avatar.SetUrl("/flames/" + tb.Text())
		e.MarkDirty(avatar)
	}, gwu.ETYPE_CHANGE)
	table.Add(avatar, 0, 2)

	// table.Add(gwu.NewLabel("Password:"), 1, 0)
	// pb := gwu.NewPasswBox("")
	// pb.Style().SetWidthPx(160)
	// table.Add(pb, 1, 1)
	p.Add(table)
	// b := gwu.NewButton("OK")
	// b.AddEHandlerFunc(func(e gwu.Event) {
	// 	if tb.Text() == "admin" && pb.Text() == "a" {
	// 		e.Session().RemoveWin(win) // Game win is removed, password will not be retrievable from the browser
	// 		buildPrivateWins(e.Session())
	// 		e.ReloadWin("main")
	// 	} else {
	// 		e.SetFocusedComp(tb)
	// 		errL.SetText("Invalid user name or password!")
	// 		e.MarkDirty(errL)
	// 	}
	// }, gwu.ETYPE_CLICK)
	// p.Add(b)
	// l = gwu.NewLabel("")
	// p.Add(l)
	// p.CellFmt(l).Style().SetHeightPx(200)

	win.Add(p)
	win.SetFocusedCompId(tb.Id())

	p = gwu.NewPanel()
	p.SetLayout(gwu.LAYOUT_HORIZONTAL)
	p.SetCellPadding(2)
	p.Add(gwu.NewLabel("Here's an ON/OFF switch which enables/disables the other one:"))
	sw := gwu.NewSwitchButton()
	sw.SetOnOff("ENB", "DISB")
	sw.SetState(true)
	p.Add(sw)
	p.Add(gwu.NewLabel("And the other one:"))
	sw2 := gwu.NewSwitchButton()
	sw2.SetEnabled(true)
	sw2.Style().SetWidthPx(100)
	p.Add(sw2)
	sw.AddEHandlerFunc(func(e gwu.Event) {
		sw2.SetEnabled(sw.State())
		e.MarkDirty(sw2)
	}, gwu.ETYPE_CLICK)
	win.Add(p)

	s.AddWin(win)
}

type Game struct{}

func (h Game) Created(s gwu.Session) {
	fmt.Println("SESSION created:", s.Id())
	buildGameWin(s)
}

func (h Game) Removed(s gwu.Session) {
	fmt.Println("SESSION removed:", s.Id())
}
