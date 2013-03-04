* logyst
** logging everywhere

- migrate local
-- db-migrate up

- migrate remote
-- heroku run bash --account fluxa
-- db-migrate up -e prod

- db info
-- heroku config --account fluxa
