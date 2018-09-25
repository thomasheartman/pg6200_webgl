let modelFromAbstract = abstract =>
  AbstractTypes.{
    Types.objData: abstract->objDataGet,
    programs: abstract->programsGet |> StringMap.fromJsDict,
    texture: abstract->textureGet,
  };

let modelToRenderArgs = (model, programName) =>
  Types.(
    AbstractTypes.renderArg(
      ~objData=model.objData,
      ~program=StringMap.find(programName, model.programs),
      ~texture=model.texture,
    )
  );

let globalOptsToAbstract = opts =>
  Types.(
    AbstractTypes.globalOptions(
      ~scale=opts.scale->Utils.toDecimal,
      ~rotation=opts.rotation->Vector.asArray,
    )
  );

/* let render = */
/*   ( */
/*   opts: AbstractTypes.globalOptions, */
/*   drawArgs: AbstractTypes.drawArgs, */
/*   draw: */
/*     (AbstractTypes.drawArgs, float, AbstractTypes.globalOptions) => unit, */
/*   rotation: float, */
/*   previousTime: float, */
/*   currentTime: float, */
/* ) => { */

let render = (opts, drawArgs, draw, rotation, previousTime, currentTime) => {
  let currentSeconds = currentTime *. 0.001;
  let delta = currentSeconds -. previousTime;
  draw(drawArgs, rotation, opts);
  (rotation +. delta, currentTime);
};
