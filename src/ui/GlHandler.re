open GlReducer;

let isSelected = name =>
  fun
  | Some(a) when a === name => true
  | _ => false;

let sortedBindings = map =>
  map
  |> StringMap.bindings
  |> List.fast_sort((a, b) => compare(fst(a), fst(b)));

let elementArray = (toElement, xs) =>
  xs
  ->sortedBindings
  ->List.map(toElement, _)
  ->Array.of_list
  ->ReasonReact.array;

let modelButtons = (send, models) =>
  elementArray(
    ((name, model)) =>
      <SpanButton
        key=name
        className={Model.shouldRender(model) ? "active" : ""}
        onClick={_ => name->ClickModel->send}
        text=name
      />,
    models,
  );

let selectModel =
  <div className="span-all alert button-style" disabled=true>
    {ReasonReact.string("Please select a model first")}
  </div>;

let ifSome = content =>
  fun
  | Some(x) => content(x)
  | None => selectModel;

let shaderButtons = (onClick, model: Model.t) =>
  elementArray(
    ((key, _drawArgs)) => {
      let isSelected = key === model.currentDrawArgs;
      <button
        key
        className={isSelected ? "active" : ""}
        disabled=isSelected
        onClick={_ => onClick(key)}>
        {ReasonReact.string(key)}
      </button>;
    },
    model.drawArgs,
  );

let rangeSlider = (value, onChange) =>
  <input
    className="button-style"
    type_="range"
    min=0
    max="200"
    value
    step=1.0
    onChange
  />;

let rotationSliders = (send, name, rotation) =>
  List.map2(
    (f, a) =>
      rangeSlider(string_of_int(a), event =>
        send(
          event
          ->Event.value
          ->int_of_string
          ->(v => Vector.update(rotation, f(v)))
          ->(x => SetRotation(name, x)),
        )
      ),
    [Vector.x, Vector.y, Vector.z],
    Vector.toList(rotation),
  )
  |> Array.of_list;

let modelTransforms = (send, name, model: Model.t) =>
  <>
    <fieldset className="span-all no-pad-h">
      <legend> {ReasonReact.string("Scale")} </legend>
      {
        rangeSlider(model.scale->string_of_int, event =>
          send(event->Event.value->int_of_string->(x => SetScale(name, x)))
        )
      }
    </fieldset>
    <fieldset className="span-all no-pad-h">
      <legend> {ReasonReact.string("Rotation (X, Y, Z)")} </legend>
      <div className="controls">
        ...{rotationSliders(send, name, model.rotation)}
      </div>
    </fieldset>
  </>;

let handleKeyDown = (send, e) => KeyDown(e)->send;
let handleKeyUp = (send, e) => KeyUp(e)->send;
let handleCanvasResize = (gl, send, e) => {
  let (width, height) = CanvasResizeEvent.details(e);
  setViewport(gl, width, height);
  send(SetAspect(float_of_int(width) /. float_of_int(height)));
};

let fieldsets = (send, state): array(Fieldset.t) =>
  Fieldset.(
    Array.concat([
      [|
        Leaf({
          content:
            <button className="span-all" onClick={_ => send(Reset)}>
              {ReasonReact.string("Reset")}
            </button>,
          legend: "",
        }),
        Leaf({content: modelButtons(send, state.models), legend: "Models"}),
      |],
      state.models
      |> StringMap.filter((_, v) => Model.shouldRender(v))
      |> sortedBindings
      |> List.map(((k, v)) =>
           Branch({
             legend: k,
             children: [|
               Leaf({
                 content:
                   shaderButtons(
                     shader => send(SelectShader(k, shader)),
                     v,
                   ),
                 legend: "Shaders",
               }),
               Leaf({content: modelTransforms(send, k, v), legend: ""}),
             |],
           })
         )
      |> Array.of_list,
    ])
  );

let component = ReasonReact.reducerComponent("GL Handler");
let make = (~glRenderingContext, _children) => {
  ...component,
  initialState: () => initialState(glRenderingContext),
  reducer,
  didMount: self => {
    Event.addKeyDownListener(handleKeyDown(self.send));
    Event.addKeyUpListener(handleKeyUp(self.send));
    CanvasResizeEvent.addListener(
      handleCanvasResize(glRenderingContext, self.send),
    );
    self.send(PrepareRender);
  },
  willUnmount: self => {
    Event.removeKeyDownListener(handleKeyDown(self.send));
    Event.removeKeyUpListener(handleKeyUp(self.send));
    CanvasResizeEvent.removeListener(
      handleCanvasResize(glRenderingContext, self.send),
    );
  },
  render: self =>
    <>
      <div className="grid-2-cols full-width">
        <fieldset>
          <legend> {ReasonReact.string("Time stamp")} </legend>
          {ReasonReact.string(self.state.nextTime |> string_of_float)}
        </fieldset>
      </div>
      <Controls contents={fieldsets(self.send, self.state)} />
    </>,
};
