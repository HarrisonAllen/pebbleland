
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
import { secret_key } from './secret_key';
import { prisma } from './prisma';
import { TOKEN_EXPIRATION } from './config'
import * as errs from "./error_handling";

// Middleware for token verification
export async function authenticate_token (req:any, res:any, next:Function) {
    const authHeader = req.headers['authorization'];
    const token = authHeader && authHeader.split(' ')[1];
  
    if (!token) return errs.no_token(res);
  
    jwt.verify(token, secret_key, (err:any, user:any) => {
        if (err) return errs.invalid_token(res);
        req.user = user;
        next();
    });
};

export async function register_user(req:any, res:any) {
    const { username, password } = req.body;

    // Hash password
    const hashed_password = await bcrypt.hash(password, 8);

    const existing_user = await prisma.testUser.findUnique({ where: { username: username } });

    if (existing_user != null) {
        return errs.user_exists(res, username);
    } 
    
    try {
        const user = await prisma.testUser.create({
            data: { 
                username: username,
                password: hashed_password
            } 
        });  
        res.status(201).json({ message:"Registration success" });
    } catch (error:any) {
        return errs.generic_error(res, "Registration failed")
    }

}

export async function login(req:any, res:any) {
    const { username, password } = req.body;

    const user = await prisma.testUser.findUnique({
        where: { username: username },
    });
    if (!user) {
        return errs.unknown_user(res, username);
    }

    if (!(await bcrypt.compare(password, user.password))) {
        return errs.invalid_credentials(res);
    }

    // Generate token
    const token = jwt.sign({ username: user.username }, secret_key, { expiresIn: TOKEN_EXPIRATION });

    res.status(200).json({ token: token });
}