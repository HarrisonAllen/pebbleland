
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
import { secret_key } from './secret_key';
import { prisma } from './prisma';
import { TOKEN_EXPIRATION } from './config'
import * as errs from "./error_handling";
import { WatchDirectoryFlags } from 'typescript';

// Middleware for token verification
export async function authenticate_token (req:any, res:any, next:Function) {
    const authHeader = req.headers['authorization'];
    const token = authHeader && authHeader.split(' ')[1];
  
    if (!token) return errs.no_token(res);
  
    jwt.verify(token, secret_key, (err:any, user_id:any) => {
        if (err) return errs.invalid_token(res);
        req.user_id = user_id;
        next();
    });
};

export async function register_user(req:any, res:any) {
    const { account_id, watch_id, password, username, email } = req.body;

    if (!password) return errs.generic_error(res, "Password required");
    if (!username) return errs.generic_error(res, "Username required");
    if (!email) return errs.generic_error(res, "Email required");

    // Hash password
    const hashed_password = await bcrypt.hash(password, 10);

    const existing_user = await prisma.user.findUnique({ 
        where: { 
            userID: {
                accountID: account_id,
                watchID: watch_id 
            }
        } 
    });

    if (existing_user != null) {
        return errs.user_exists(res);
    } 
    
    try {
        const user = await prisma.user.create({
            data: { 
                accountID: account_id,
                watchID: watch_id,
                accountInfo: {
                    create: {
                        password: hashed_password,
                        email: email
                    }
                },
                playerInfo: {
                    create: {
                        username: username
                    }
                }
            } 
        });  
        res.status(201).json({ message:"Registration success" });
    } catch (error:any) {
        console.log(error);
        return errs.generic_error(res, "Registration failed")
    }

}

export async function login(req:any, res:any) {
    const { account_id, watch_id, password } = req.body;

    const user = await prisma.user.findUnique({ 
        where: { 
            userID: {
                accountID: account_id,
                watchID: watch_id 
            }
        },
        select: {
            accountInfo: {
                select: {
                    password: true
                }
            }
        }
    });
    if (!user) {
        return errs.unknown_user(res);
    }

    if (!(await bcrypt.compare(password, user.accountInfo.password))) {
        return errs.invalid_credentials(res);
    }

    // Generate token
    const token = jwt.sign({ account_id:account_id, watch_id:watch_id }, secret_key, { expiresIn: TOKEN_EXPIRATION });

    res.status(200).json({ token: token });
}