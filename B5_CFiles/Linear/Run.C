void Run() {
  gROOT->ProcessLine(".L b5.C");
  gROOT->ProcessLine("b5 t");
  gROOT->ProcessLine("t.Loop()");
}