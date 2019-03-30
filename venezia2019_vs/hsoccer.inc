mesh {
  texture {WhiteFaceTexture}
}
mesh {
  texture {BlackFaceTexture}
}
union {
  cylinder {<0.349004,-0.113304,0.296483>,<0.40689,-0.0416944,-0.237053>,BorderThickness}
  cylinder {<0.349004,-0.113304,0.296483>,<-0.147215,0.0705635,0.439468>,BorderThickness}
  cylinder {<-0.147215,0.0705635,0.439468>,<-0.392944,0.259306,-0.00477574>,BorderThickness}
  cylinder {<-0.392944,0.259306,-0.00477574>,<-0.0525197,0.189419,-0.42571>,BorderThickness}
  cylinder {<-0.0525197,0.189419,-0.42571>,<0.40689,-0.0416944,-0.237053>,BorderThickness}
  texture {BorderTexture}
}
