open Utils;
include GlHandlerModel;

type shaderKey = string;

type action =
  | SelectShader(modelName, shaderKey)
  | SelectModel(modelName)
  | KeyPress(Webapi.Dom.KeyboardEvent.t)
  | SetScale(int)
  | SetRotation(Vector.t(int))
  | SetCamera(Movement.t)
  | SetRafId(option(Webapi.rafId))
  | PrepareRender
  | Render(AbstractTypes.drawArgs, AbstractTypes.globalOptions, float);

let getRenderArg = (models, programs, name) =>
  Model.toRenderArgs(
    StringMap.find(name, models),
    StringMap.find(name, programs),
  );

let cancelAnimation =
  fun
  | Some(id) => Webapi.cancelAnimationFrame(id)
  | None => ();

let handleCameraUpdate = (mvmt, camera: Camera.t) => {
  open Movement;
  let (vec, axis, fill) =
    switch (mvmt) {
    | Translation(axis) => (
        camera.position,
        axis,
        (position => {...camera, position}),
      )
    | Rotation(axis) => (
        camera.rotation,
        axis,
        (rotation => {...camera, rotation}),
      )
    };
  camera.velocity |> Input.move(axis) |> Vector.addSome(vec) |> fill;
};

let reducer = (action, state: state) =>
  switch (action) {
  | Render(drawArgs, opts, currentTime) =>
    ReasonReact.UpdateWithSideEffects(
      {...state, nextTime: currentTime, previousTime: state.nextTime},
      (
        self => {
          drawScene(drawArgs, state.nextTime, opts);
          Webapi.requestCancellableAnimationFrame(x =>
            self.send(Render(drawArgs, opts, x *. 0.001))
          )
          ->Some
          ->SetRafId
          |> self.send;
        }
      ),
    )

  | PrepareRender =>
    ReasonReact.SideEffects(
      (
        self => {
          cancelAnimation(state.rafId);
          self.send(SetRafId(None));
          switch (state.model) {
          | Some(name) =>
            self.send(
              Render(
                name->StringMap.find(state.drawArgs),
                state.globalOptions |> GlobalOptions.toAbstract,
                state.nextTime,
              ),
            )
          | None => state.clear()
          };
        }
      ),
    )

  | KeyPress(e) =>
    switch (Input.getMovement(Webapi.Dom.KeyboardEvent.code(e))) {
    | Some(mvmt) =>
      ReasonReact.SideEffects((self => self.send(SetCamera(mvmt))))
    | None => ReasonReact.NoUpdate
    }

  | SetCamera(mvmt) =>
    ReasonReact.UpdateWithSideEffects(
      {
        ...state,
        globalOptions: {
          ...state.globalOptions,
          camera: handleCameraUpdate(mvmt, state.globalOptions.camera),
        },
      },
      (self => self.send(PrepareRender)),
    )

  | SelectModel(name) =>
    let nextState =
      switch (state.model) {
      | Some(n) when n === name => {...state, model: None}
      | _ =>
        let selectedPrograms =
          StringMap.update(
            name,
            default(defaultProgram),
            state.selectedPrograms,
          );
        let drawArgs =
          StringMap.add(
            name,
            getRenderArg(state.models, selectedPrograms, name)
            |> state.getDrawArgs,
            state.drawArgs,
          );
        {...state, model: Some(name), selectedPrograms, drawArgs};
      };
    ReasonReact.UpdateWithSideEffects(
      nextState,
      (self => self.send(PrepareRender)),
    );

  | SetRotation(rotation) =>
    ReasonReact.UpdateWithSideEffects(
      {
        ...state,
        globalOptions: {
          ...state.globalOptions,
          rotation,
        },
      },
      (self => self.send(PrepareRender)),
    )

  | SetScale(v) =>
    ReasonReact.UpdateWithSideEffects(
      {
        ...state,
        globalOptions: {
          ...state.globalOptions,
          scale: v,
        },
      },
      (self => self.send(PrepareRender)),
    )

  | SetRafId(id) => ReasonReact.Update({...state, rafId: id})

  | SelectShader(modelName, programName) =>
    ReasonReact.UpdateWithSideEffects(
      {
        let selectedPrograms =
          StringMap.add(modelName, programName, state.selectedPrograms);
        let drawArgs =
          StringMap.add(
            modelName,
            getRenderArg(state.models, selectedPrograms, modelName)
            |> state.getDrawArgs,
            state.drawArgs,
          );
        {...state, selectedPrograms, drawArgs};
      },
      (self => self.send(PrepareRender)),
    )
  };