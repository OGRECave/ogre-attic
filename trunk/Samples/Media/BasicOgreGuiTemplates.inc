// Ogre overlay scripts
template container BorderPanel(SS/Templates/BasicBorderPanel)
{
	left 0
	top 0
	width 1
	height 1
	material Core/StatsBlockCenter
    border_size 0.06 0.06 0.06665 0.06665
    border_material Core/StatsBlockBorder
    border_topleft_uv     0.0000 1.0000 0.1914 0.7969
	border_top_uv         0.1914 1.0000 0.8086 0.7969
	border_topright_uv    0.8086 1.0000 1.0000 0.7969
	border_left_uv        0.0000 0.7969 0.1914 0.2148
	border_right_uv       0.8086 0.7969 1.0000 0.2148
	border_bottomleft_uv  0.0000 0.2148 0.1914 0.0000
    border_bottom_uv      0.1914 0.2148 0.8086 0.0000
	border_bottomright_uv 0.8086 0.2148 1.0000 0.0000
}

template container BorderPanel(SS/Templates/BasicSmallBorderPanel) : SS/Templates/BasicBorderPanel
{
    border_size 0.04 0.04 0.04665 0.04665
}


template element TextArea(SS/Templates/BasicText)
{
	font_name Ogre
	char_height 0.08
	colour_top 1 1 0
	colour_bottom 1 0.2 0.2
	left 0.03
	top 0.02
	width 0.12
	height 0.09
}

template container TextBox(SS/Templates/BasicTextBox) 
{ 
   left 0.02 
   top 0.01 
   width 0.3 
   height 0.1 
   text_area SS/Templates/BasicText test 
   back_panel SS/Templates/BasicSmallBorderPanel 
}

template container BorderButton(SS/Templates/BasicButton) : SS/Templates/BasicBorderPanel
{
	left 0.82
	top 0.45
	width 0.16
	height 0.13
	material Core/StatsBlockCenter
	border_up_material Core/StatsBlockBorder/Up
	border_down_material Core/StatsBlockBorder/Down
}

template container BorderButton(SS/Templates/BasicSmallButton) : SS/Templates/BasicButton
{
    border_size 0.02 0.02 0.02665 0.02665
}

template container Panel(SS/Templates/BasicPanel)
{
	left 0 
	top 0
	width 1
	height 1
	material Core/StatsBlockCenter
}

template container Button(SS/Templates/ScrollBar/UpButton) : SS/Templates/BasicPanel
{
	left 0 
	top 0
	width 1
	height 1
	material Examples/Scrollbar/Up
	button_up_material Examples/Scrollbar/Up
	button_down_material Examples/Scrollbar/Up/Pressed
}

template container Button(SS/Templates/ScrollBar/DownButton) : SS/Templates/BasicPanel
{
	left 0 
	top 0
	width 1
	height 1
	material Examples/Scrollbar/Down
	button_up_material Examples/Scrollbar/Down
	button_down_material Examples/Scrollbar/Down/Pressed
}

template container Panel(SS/Templates/ScrollBar/ScrollBit) 
{
	material Examples/Scrollbar/Thumb
}

template container ScrollBar(SS/Templates/ScrollBar)
{
	width 0.05
	material Examples/Scrollbar/Background
	up_button SS/Templates/ScrollBar/UpButton
	down_button SS/Templates/ScrollBar/DownButton
	scroll_bit SS/Templates/ScrollBar/ScrollBit
}
