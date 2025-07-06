import * as auth from "./authentication"
import * as ureqs from "./user_requests"
import { app } from "./express";

export function set_up_routes() {
    // TODO: error handling
    app.get('/users', async (req:any, res:any) => { 
        ureqs.get_users(req, res) 
    });
    
    // Protected route example
    app.get('/my_info', auth.authenticate_token, (req:any, res:any) => {
        ureqs.get_my_info(req, res);
    });
    
    // Protected route example
    app.get('/my_fave_num', auth.authenticate_token, (req:any, res:any) => {
        res.status(200).send('My favorite number is ' + req.user.favoriteNumber);
    });
    
    // Protected route example
    app.post('/set_my_fave_num', auth.authenticate_token, (req:any, res:any) => {
        ureqs.set_fave_num(req, res);
    });
    
    app.post('/post-test', (req:any, res:any) => {
        // Access the body from the request
        const requestBody = req.body;
    
        // Respond with the data from the request body
        res.send(`Received the following data: ${JSON.stringify(requestBody)}`);
    });
    
    // AUTHENTICATION
    
    app.post('/signup', (req:any, res:any) => auth.register_user(req, res));
    
    app.post('/login', (req:any, res:any) => auth.login(req, res));
}