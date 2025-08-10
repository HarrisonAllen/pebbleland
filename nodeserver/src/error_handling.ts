export const ERROR_CODES = Object.freeze({
    // auth
    no_token: 4000,
    invalid_token: 4001,
    invalid_credentials: 4002,

    // lookup
    unknown_user: 4003,
    user_exists: 4004,

    // other
    generic: 4099
});

export function no_token(res:any) {
    res.status(401).json({
        error: ERROR_CODES.no_token,
    });
}

export function invalid_token(res:any) {
    res.status(403).json({
        error: ERROR_CODES.invalid_token,
    });
}

export function invalid_credentials(res:any) {
    res.status(401).json({
        error: ERROR_CODES.invalid_credentials,
    });
}

export function unknown_user(res:any) {
    res.status(400).json({
        error: ERROR_CODES.unknown_user
    });
}

export function user_exists(res:any) {
    res.status(400).json({
        error: ERROR_CODES.user_exists
    });
}

export function generic_error(res:any, message:string) {
    res.status(400).json({
        error: ERROR_CODES.unknown_user,
        message: message
    });
}