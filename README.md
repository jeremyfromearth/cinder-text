# cinder-text
> *A text layout and render component for Cinder.*


Admittedly this project does not aim to be completely typographically correct, though that may change in the future. The main aim of this project is to provide a combination of features (namely leading, wrapping, word spacing and measurability) that the other various Cinder classes do not accomodate.

Cinder ships with three classes for drawing text. Below is a feature matrix for the three native Cinder text rendering classes and this project.

|class      |wrapping|leading|word-space|measurable|append text|append line|clear text|multiple fonts|resizable|align    |
|-----------|--------|-------|----------|----------|-----------|-----------|----------|--------------|---------|---------|
|TextBox    | yes    |no     |no        |yes       |yes        |no         |yes       |no            |yes      |l,r,c    |
|TextLayout | no     |yes    |no        |no        |yes        |yes        |no        |yes           |yes      |l,r,c    |
|TextureFont| yes    |no     |no        |yes       |no         |no         |no        |no            |yes      |none     |
|CinderText | yes    |yes    |yes       |yes       |yes        |yes        |yes       |no            |yes      |l,r      |


