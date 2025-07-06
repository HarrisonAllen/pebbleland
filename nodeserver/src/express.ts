const express = require('express');

export const app = express();
export function set_up_express() {
    app.use(express.json());
}