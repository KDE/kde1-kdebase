#ifndef kioerrors_h
#define kioerrors_h

#define KIO_ERROR_CouldNotWrite 1
#define KIO_ERROR_CouldNotRead 2
#define KIO_ERROR_CouldNotDelete 3
#define KIO_ERROR_CouldNotCreateSocket 4
#define KIO_ERROR_UnknownHost 5
#define KIO_ERROR_CouldNotConnect 7
#define KIO_ERROR_CouldNotMount 8
#define KIO_ERROR_CouldNotUnmount 9
#define KIO_ERROR_NotImplemented 10
#define KIO_ERROR_CouldNotLogin 11
#define KIO_ERROR_MalformedURL 12
#define KIO_ERROR_CouldNotMkdir 13
#define KIO_ERROR_CouldNotList 14
#define KIO_ERROR_TarError 15
#define KIO_ERROR_GzipError 16
#define KIO_ERROR_FileExists 17
/**
 * Use this if it is technically impossible to fullfill the
 * request. For example it is impossible to list the directory
 * structure of a GZIP file.
 */
#define KIO_ERROR_NotPossible 18
#define KIO_ERROR_SlaveDied 19
#define KIO_ERROR_FileDoesNotExist 20
#endif
