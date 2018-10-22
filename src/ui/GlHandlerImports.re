[@bs.module "../models"]
external getModels:
  WebGl.renderingContext => Js.Dict.t(Js.Dict.t(DrawArgs.abstract)) =
  "default";

type architecture;

[@bs.module "../models"]
external getArchitecture: WebGl.renderingContext => architecture =
  "architecture";

type aspect = float;
[@bs.module "../renderUtils"]
external drawScene:
  (
    WebGl.renderingContext,
    architecture,
    array(Model.abstract),
    Camera.t,
    aspect,
    array(int),
    float
  ) =>
  unit =
  "";

[@bs.module "../glUtils"]
external getAspect: WebGl.renderingContext => aspect = "";

[@bs.module "../glUtils"]
external setViewport: (WebGl.renderingContext, int, int) => unit = "";

[@bs.module "../renderUtils"]
external drawEmptyScene: WebGl.renderingContext => unit = "";
