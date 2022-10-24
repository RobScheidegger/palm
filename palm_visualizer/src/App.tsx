import React, { useEffect } from "react";
import logo from "./logo.svg";
import "./App.css";
import { useRobot } from "./state/useRobot";

function App() {
  useEffect(() => {
    setInterval(() => {}, 100);
  }, []);

  return <RobotVisualizer />;
}

export default App;
