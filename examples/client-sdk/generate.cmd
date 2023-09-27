set JAVA_OPTS=%JAVA_OPTS% -Dio.swagger.parser.util.RemoteUrl.trustAll=true
set JAVA_OPTS=%JAVA_OPTS% -Dio.swagger.v3.parser.util.RemoteUrl.trustAll=true

@RMDIR /S /Q python
@MKDIR python
java %JAVA_OPTS% -jar openapi-generator-cli.jar generate -i https://127.0.0.1:9000/api-docs/oas-3.0.0.json -g python -o python --package-name=moonshine --skip-validate-spec